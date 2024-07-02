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


#ifndef _NETADAPTER_H
#define _NETADAPTER_H

#include <deque>
#include <string>
#include <stdint.h>

#include <hbk/communication/ipv4address.h>
#include <hbk/communication/ipv6address.h>

namespace hbk {
	namespace communication {
		enum enResult {
			ERR_SUCCESS = 0,
			ERR_NO_SUCCESS = -1,
			ERR_INVALIDADAPTER = -2,
			ERR_INVALIDIPADDRESS = -3,
			ERR_INVALIDCONFIGMETHOD = -4,
			WARN_RESTART_REQUIRED = 4,
			WARN_INVALIDCONFIGMETHOD = 5
		};

		// we use a double ended queue here because we might insert to the front or to the back.
		using AddressesWithPrefix = std::deque < Ipv6Address >;
		using AddressesWithNetmask = std::deque < Ipv4Address > ;


		/// Everything about a network adapter
		class Netadapter
		{
		public:


			Netadapter();

			/// \return interface name
			std::string getName() const { return m_name; }

			/// \return all ipv4 addresses of the interface
			const AddressesWithNetmask& getIpv4Addresses() const
			{
				return m_ipv4Addresses;
			}

			/// \return all ipv6 addresses of the interface
			const AddressesWithPrefix& getIpv6Addresses() const
			{
				return m_ipv6Addresses;
			}

			/// \return formatted MAC address in HEX punctuated with ":" and upper case letters
			std::string getMacAddressString() const { return m_macAddress; }

			/// \return 0 for non FireWire adapters
			uint64_t getFwGuid() const
			{
				return m_fwGuid;
			}

			/// \return interface index
			unsigned int getIndex() const
			{
				return m_index;
			}

			/// \return interface index of master interface. -1 when there is no master interface
			int getMasterIndex() const
			{
				return m_masterIndex;
			}

			/// \return true if this is a firewire adapter
			bool isFirewireAdapter() const;

			/// If interfaces are configured using DHCP, another default gateway might be used.
			/// \return Address of the manual ipv4 default gateway, empty when there is none.
			static std::string getIpv4DefaultGateway();

			/// interface name
			std::string m_name;

			/// all ipv4 addresses of the interface
			AddressesWithNetmask m_ipv4Addresses;
			/// all ipv6 addresses of the interface
			AddressesWithPrefix m_ipv6Addresses;

			/// Unique identifier of the ethernet interface
			std::string m_macAddress;

			/// Unique identifier of the firewire interface
			uint64_t m_fwGuid;

			/// interface index
			unsigned int m_index;

			/// interface index of master device. -1 when there is no master
			int m_masterIndex;
		};
	}
}
#endif
