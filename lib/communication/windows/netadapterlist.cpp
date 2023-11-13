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

#include <iomanip>
#include <sstream>
#include <string>
#include <stdint.h>
#include <iterator>
#include <mutex>
#include <cstring>

#include <windows.h>
#include <iphlpapi.h>
#define syslog fprintf
#define LOG_ERR stderr

#include "hbk/exception/exception.hpp"

#include "hbk/communication/netadapterlist.h"
#include "hbk/communication/netadapter.h"

namespace hbk {
	namespace communication {
		NetadapterList::NetadapterList()
		{
			update();
		}

		void NetadapterList::update()
		{
			IP_ADAPTER_INFO* pAdptInfo = nullptr;
			IP_ADAPTER_INFO* pNextAd = nullptr;
			ULONG ulLen = 0;
			DWORD erradapt;

			//This call returns the number of network adapter in ulLen
			erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);

			if (erradapt == ERROR_BUFFER_OVERFLOW) {
				pAdptInfo = reinterpret_cast < IP_ADAPTER_INFO* >(new UINT8[ulLen]);
				erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);
			}

			if (erradapt == ERROR_SUCCESS) {
				std::lock_guard < std::mutex > lock(m_adaptersMtx);
				m_adapters.clear();
				// initialize the pointer we use the move through
				// the list.
				pNextAd = pAdptInfo;

				// loop through for all available interfaces and setup an associated
				// CNetworkAdapter class.
				while (pNextAd) {
					std::stringstream macStream;
					Netadapter Adapt;
					std::vector < std::string > GatewayList;
					IP_ADDR_STRING* pNext	= nullptr;

					unsigned int adapterIndex = pNextAd->Index;
					Adapt.m_index = adapterIndex;
					Adapt.m_macAddress.clear();

					for (unsigned int i = 0; i < pNextAd->AddressLength; i++) {
						if (i > 0) {
							macStream << ":";
						}

						macStream << std::hex << std::setw(2) << std::setfill('0') << static_cast < unsigned int >(pNextAd->Address[i]) << std::dec;
					}

					Adapt.m_macAddress = macStream.str();

					Ipv4Address addressWithNetmask;

					if (pNextAd->CurrentIpAddress) {
						addressWithNetmask.address = pNextAd->CurrentIpAddress->IpAddress.String;
						addressWithNetmask.netmask = pNextAd->CurrentIpAddress->IpMask.String;
					} else {
						addressWithNetmask.address = pNextAd->IpAddressList.IpAddress.String;
						addressWithNetmask.netmask = pNextAd->IpAddressList.IpMask.String;
					}

					// there might be several addresses per interface
					if (addressWithNetmask.address != "0.0.0.0") {
						// 0.0.0.0 does mean "no address"
						Adapt.m_ipv4Addresses.push_back(addressWithNetmask);
					}

					// an adapter usually has just one gateway however the provision exists
					// for more than one so to "play" as nice as possible we allow for it here
					// as well.
					pNext = &(pNextAd->GatewayList);

					while (pNext) {
						GatewayList.push_back(pNext->IpAddress.String);
						pNext = pNext->Next;
					}


					Adapt.m_name = pNextAd->Description;
					if ((Adapt.getIpv4Addresses().empty() == false) || (Adapt.getIpv6Addresses().empty() == false)) {
						// T-C software does not want interfaces without address!
						m_adapters[adapterIndex] = Adapt;
					}

					// move forward to the next adapter in the list so
					// that we can collect its information.
					pNextAd = pNextAd->Next;
				}
			}

			// free any memory we allocated from the heap before exit.
			delete[] pAdptInfo;
		}


		NetadapterList::Adapters NetadapterList::get() const
		{
			std::lock_guard < std::mutex > lock(m_adaptersMtx);
			return m_adapters;
		}

		NetadapterList::AdapterArray NetadapterList::getArray() const
		{
			std::lock_guard < std::mutex > lock(m_adaptersMtx);
			AdapterArray result;
			result.reserve(m_adapters.size());

			for(Adapters::const_iterator iter = m_adapters.begin(); iter!=m_adapters.end(); ++iter) {
				result.push_back(iter->second);
			}

			return result;
		}

		Netadapter NetadapterList::getAdapterByName(const std::string& adapterName) const
		{
			std::lock_guard < std::mutex > lock(m_adaptersMtx);

			for (Adapters::const_iterator iter = m_adapters.begin(); iter != m_adapters.end(); ++iter) {
				if (iter->second.getName().compare(adapterName) == 0) {
					return iter->second;
				}
			}

			throw hbk::exception::exception("invalid interface");
			// unreachable: return Netadapter();
		}

		Netadapter NetadapterList::getAdapterByInterfaceIndex(unsigned int interfaceIndex) const
		{
			std::lock_guard < std::mutex > lock(m_adaptersMtx);

			Adapters::const_iterator iter = m_adapters.find(interfaceIndex);
			if(iter==m_adapters.end()) {
				throw hbk::exception::exception("invalid interface");
			}

			return iter->second;
		}
		
                std::string NetadapterList::checkSubnet(const std::string& excludeAdapterName, const communication::Ipv4Address& requestedAddress) const
                {
                        std::string requestedSubnet = requestedAddress.getSubnet();

                        for (communication::NetadapterList::Adapters::const_iterator adapterIter=m_adapters.begin(); adapterIter!=m_adapters.end(); ++adapterIter ) {
                                const communication::Netadapter& adapter = adapterIter->second;
                                if (excludeAdapterName != adapter.getName()) {
                                        communication::AddressesWithNetmask addresses = adapter.getIpv4Addresses();

                                        for (communication::AddressesWithNetmask::const_iterator addressIter = addresses.begin(); addressIter!=addresses.end(); ++addressIter) {
                                                const communication::Ipv4Address& address = *addressIter;
                                                if (requestedSubnet==address.getSubnet()) {
                                                        return adapter.getName();
                                                }
                                        }
                                }
                        }
                        return "";
                }
	}
}


