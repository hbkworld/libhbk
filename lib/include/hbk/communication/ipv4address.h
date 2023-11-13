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



#ifndef _IPV4ADDRESS_T_H
#define _IPV4ADDRESS_T_H


#include <string>

namespace hbk {
	namespace communication {
		/// Tools concerning ipv4 addresses
		struct Ipv4Address {
			/// \return if ipv4 address and subnet mask are equal
			bool equal(const struct Ipv4Address& op) const;

			/// \return subnet resulting of address and netmask
			std::string getSubnet() const;

			/// \return if address is in 169.254.0.0/16
			static bool isApipaAddress(const std::string& address);
			/// some address ranges are reserved and may not be used
			/// (see https://en.wikipedia.org/wiki/Reserved_IP_addresses)
			static bool isValidManualAddress(const std::string& ip);
			
			/// \return true if ip is a valid ipv4 subnet mask
			static bool isValidNetmask(const std::string& ip);

			/// \return prefix corresponding to the given netmaks
			/// gaps are not allowed!
			static int getPrefixFromNetmask(const std::string& netmask);
			
			/// \return netmask corresponding to the given prefix
			static std::string getNetmaskFromPrefix(unsigned int prefix);

			/// the address as string xxx.xxx.xxx.xxx
			std::string address;
			/// the subnet mask as string xxx.xxx.xxx.xxx
			std::string netmask;

			/// \return if ipv4 address and subnet mask are equal
			bool operator==(const Ipv4Address& op) const;
		};
	} // namespace communication
} // namespace hbk
#endif
