// This code is licenced under the MIT license:
//
// Copyright (c) 2024 Hottinger Brüel & Kjær
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <algorithm>
#include <cstring>

#ifdef _WIN32
#ifndef _WINSOCK2API_
#include <ws2tcpip.h>
#endif
#else
#include <sys/types.h>
#include <arpa/inet.h>
#endif

#include <sstream>


#include "hbk/communication/ipv6address.h"
#include "hbk/string/split.h"


namespace hbk {
	namespace communication {
		Ipv6Address::Ipv6Address()
			: prefix(0)
		{
		}

		bool Ipv6Address::equal(const struct Ipv6Address& op) const
		{
			if (prefix!=op.prefix) {
				return false;
			}
			
			// It seems to be to most simple to convert to binary form an back both addresses and compare both results.
			struct in6_addr addrStruct;
			struct in6_addr opAddrStruct;
			if (inet_pton(AF_INET6, address.c_str(), &addrStruct) == 0) {
				return false;
			}
			if (inet_pton(AF_INET6, op.address.c_str(), &opAddrStruct) == 0) {
				return false;
			}
			
			char addressBack[64];
			char opAddressBack[64];
						
			inet_ntop(AF_INET6, &addrStruct, addressBack, sizeof(addressBack));
			inet_ntop(AF_INET6, &opAddrStruct, opAddressBack, sizeof(opAddressBack));
			
			return !strncmp(addressBack, opAddressBack, sizeof(addressBack));
		}

		std::string Ipv6Address::getIpv4MappedAddress(const std::string& address)
		{
			static const std::string hybridIpv4AddressPrefix = "::ffff:";
			std::string addressPrefix = address.substr(0, hybridIpv4AddressPrefix.length());

			if (addressPrefix!=hybridIpv4AddressPrefix) {
				return "";
			}

			std::string ipv4Address = address.substr(hybridIpv4AddressPrefix.length());
			auto count = std::count(ipv4Address.begin(), ipv4Address.end(), '.');
			if (count!=3) {
				return "";
			}
#ifdef _WIN32
			struct in_addr inSubnet;
			if (inet_pton(AF_INET, ipv4Address.c_str(), &inSubnet)!=1) {
//			unsigned long addr = inet_addr(ipv4Address.c_str());
//			if (addr == INADDR_NONE) {
				return "";
			}
#else
			struct in_addr inSubnet;
			if (inet_aton(ipv4Address.c_str(), &inSubnet) == 0) {
				return "";
			}
#endif
			return ipv4Address;
		}
		
		std::string Ipv6Address::linkLocalFromMac(const std::string& macAddress)
		{
			hbk::string::tokens tokens = hbk::string::split(macAddress, ':');
			if (tokens.size() != 6) {
				throw std::runtime_error("not a valid MAC address");
			}
			
			unsigned int part = std::stoul(tokens[0], 0, 16);
			part ^= 1 << 1;
			std::stringstream linkLocalAddressStream;
			linkLocalAddressStream << std::hex << "fe80::" << part << tokens[1] << ":" << tokens[2] << "ff:fe" << tokens[3] << ":" << tokens[4] << tokens[5];
			return linkLocalAddressStream.str();
		}

		bool Ipv6Address::isLinkLocalAddress(const std::string& address)
		{
			static const std::string ipv6LinkLocalNet("fe80::");
			struct in6_addr addr;

/*
#ifdef _WIN32
			if (inet_pton_forWindowsxp(AF_INET6, address.c_str(), &addr) == 0) {
				return false;
			}
#else
*/
			if (inet_pton(AF_INET6, address.c_str(), &addr) == 0) {
				return false;
			}
//#endif

			std::string prefix = address.substr(0, ipv6LinkLocalNet.length());
			std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
			if (prefix!=ipv6LinkLocalNet) {
				return false;
			}
			return true;
		}
	}
}

