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
#include <cstdint>
#include <string>

#ifdef _WIN32
#ifndef _WINSOCK2API_
#include <ws2tcpip.h>
#endif
#else
#include <arpa/inet.h>
#endif

#include <hbk/string/split.h>

#include "hbk/communication/ipv4address.h"

namespace hbk {
	namespace communication {
		bool Ipv4Address::equal(const struct Ipv4Address& op) const
		{
			if( (address==op.address) && (netmask==op.netmask) ) {
				return true;
			}
			return false;
		}

		bool Ipv4Address::isApipaAddress(const std::string& address)
		{
			static const std::string apipaNet("169.254");

			auto count = std::count(address.begin(), address.end(), '.');
			if (count!=3) {
				return false;
			}
			struct in_addr inSubnet;
			if (inet_pton(AF_INET, address.c_str(), &inSubnet) != 1) {
				return false;
			}

			return (address.find(apipaNet)==0);
		}

		bool Ipv4Address::isValidManualAddress(const std::string& ip)
		{
			auto count = std::count(ip.begin(), ip.end(), '.');
			if (count!=3) {
				return false;
			}

			struct in_addr inSubnet;
			if (inet_pton(AF_INET, ip.c_str(), &inSubnet) != 1) {
				return false;
			}
			uint32_t bigAddress = htonl(inSubnet.s_addr);
			if (bigAddress==0) {
				// "0.0.0.0" is allowed and means "non-routable meta-address"
				return true;
			}

			// check for some reserved ranges
			uint8_t upperMost = bigAddress >> 24;
			if (upperMost == 0) {
				return false;
			} if (upperMost == 127) {
				// Loopback and diagnostics
				return false;
			} if (upperMost >= 224) {
				// 224 - 239: Reserved for Multicasting
				// 240 - 254: Experimental; used for research
				return false;
			}

			if(isApipaAddress(ip)) {
				return false;
			}

			return true;
		}

		bool Ipv4Address::isValidNetmask(const std::string& ip)
		{
			auto count = std::count(ip.begin(), ip.end(), '.');
			if (count!=3) {
				return false;
			}

			struct in_addr inSubnet;
			if (inet_pton(AF_INET, ip.c_str(), &inSubnet) != 1) {
				return false;
			}
			uint32_t bigAddress = htonl(inSubnet.s_addr);

			if (bigAddress == 0){
				// 0.0.0.0
				return false;
			} if (bigAddress == 0xffffffff) {
				// 255.255.255.255
				return false;
			}

			return true;
		}

		std::string Ipv4Address::getSubnet() const
		{
			std::string subnet;
			unsigned int addressByte;
			unsigned int netmaskByte;
			unsigned int subnetByte;
			hbk::string::tokens addressParts = hbk::string::split(address, '.');
			hbk::string::tokens netmaskParts = hbk::string::split(netmask, '.');
			if ((addressParts.size()!=4)||(netmaskParts.size()!=4)) {
				return "";
			}

			for (unsigned int index=0; index<4; ++index) {
				addressByte = std::stoul(addressParts[index], nullptr, 10);
				netmaskByte = std::stoul(netmaskParts[index], nullptr, 10);
				if ((addressByte>255) || (netmaskByte>255)) {
					// not a valid address!
					return "";
				}
				subnetByte = addressByte & netmaskByte;
				subnet += std::to_string(subnetByte) + '.';
			}
			subnet.pop_back(); // remove trailing '.'
			return subnet;
		}

		std::string Ipv4Address::getNetmaskFromPrefix(unsigned int prefix)
		{
			if (prefix>32) {
				// invalid
				return "";
			}
			unsigned int subnet = 0;
			for (unsigned int count=0; count<32; ++count) {
				subnet <<= 1;
				if (prefix) {
					subnet |= 1;
					--prefix;
				}
			}

			struct in_addr ip_addr = { 0 };
			ip_addr.s_addr = htonl(subnet);
			return inet_ntoa(ip_addr);
		}

		int Ipv4Address::getPrefixFromNetmask(const std::string& netmask)
		{
			unsigned int prefix = 0;
			unsigned int mask = 0x80000000;


			struct in_addr inAddr;
			if (inet_pton(AF_INET, netmask.c_str(), &inAddr) != 1) {
				//255.255.255.255 is valid!
				if (netmask != "255.255.255.255") {
					return -1;
				}
			}
			uint32_t ipv4Subnetmask = ntohl(inAddr.s_addr);
			do {
				if (ipv4Subnetmask & mask) {
					mask >>= 1;
					++prefix;
				} else {
					break;
				}
			} while(mask!=0);

			// check for following gaps which are not allowed!
			for(unsigned int pos=prefix+1; pos<32; ++pos) {
				mask >>= 1;
				if (ipv4Subnetmask & mask) {
					return -1;
				}
			}
			return static_cast < int > (prefix);
		}

		bool Ipv4Address::operator == (const Ipv4Address& op) const
		{
			return equal(op);
		}
	} // namespace communication
} // namespace hbk

