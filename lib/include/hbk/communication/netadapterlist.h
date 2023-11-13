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
//
#ifndef _HBK__NETADAPTERLIST_H
#define _HBK__NETADAPTERLIST_H

#include <vector>
#include <map>
#include <string>
#include <mutex>

#include "netadapter.h"

namespace hbk {
	namespace communication {
		/// Informationen ueber alle verfuegbaren IP-Schnittstellen.
		class NetadapterList
		{
		public:
			/// interface index is the key
			using Adapters = std::map < unsigned int, Netadapter >;
			/// all network adapters as vector
			using AdapterArray = std::vector < Netadapter > ;

			NetadapterList();

			/// \return A map with all network adapters
			Adapters get() const;

			/// the same order as returned by get()
			AdapterArray getArray() const;

			/// \throws hbk::exception
			Netadapter getAdapterByName(const std::string& adapterName) const;

			/// get adapter by interface index
			/// \throws hbk::exception
			Netadapter getAdapterByInterfaceIndex(unsigned int interfaceIndex) const;

			/// check whether subnet of requested address is already occupied by an address of an interface
			/// \param excludeAdapterName Name of an adapter to exclude from check. This is usefull to allow collision on this adapter before changing its address. Leave empty if no interface is to be excluded.
			/// \param requestedAddress the address
			/// \return name of the occupying interface or an empty string
			std::string checkSubnet(const std::string& excludeAdapterName, const communication::Ipv4Address& requestedAddress) const;

			/// force update of the adapter list
			void update();

		private:

			Adapters m_adapters;
			mutable std::mutex m_adaptersMtx;
		};
	}
}
#endif
