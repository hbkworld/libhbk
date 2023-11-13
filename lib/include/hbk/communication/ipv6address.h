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


#ifndef _IPV6ADDRESS_T_H
#define _IPV6ADDRESS_T_H


#include <string>

namespace hbk {
	namespace communication {
		/// Tools concerning ipv4 addresses
		struct Ipv6Address {
			Ipv6Address();
			
			/// ipv6 addresses are case insensitive!
			/// Leading zeroes are to be ignored!
			/// \return if ipv6 address and prefix are equal
			bool equal(const Ipv6Address &op) const;
			
			/// \return if address is a valid ipv6 address with the prefix "fe80::"
			static bool isLinkLocalAddress(const std::string& address);
			
			/// \return an empty string if address is not a valid ipv4 mapped ipv6 address
			static std::string getIpv4MappedAddress(const std::string& address);
			
			/// In the standard, IPv6 link local addresses have the following format:
			/// \codeline fe80::<EUI-64 representation of the MAC address>
			/// 
			/// Client systems such as Ubuntu Desktop will use random addresses
			/// instead to avoid tracking their activity.
			/// 
			/// When using systemd-networkd, IPv6LinkLocalAddressGenerationMode configures
			/// how IPv6 link local address are generated.
			///
			/// \param 48 bit MAC address as string
			/// \example cA:00:0e:74:00:08 => fe80::c800:0eff:fe74:0008
			/// \throws std::runtime_error
			static std::string linkLocalFromMac(const std::string& macAddress);
			
			/// the address as string i.e fe80::999c:a84:2f22:1ccd
			std::string address;
			/// the network prefix (number of bits leftmost desribing the network)
			unsigned int prefix;
		};
	}
}
#endif
