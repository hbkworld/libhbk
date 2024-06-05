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

#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <syslog.h>

#include "hbk/communication/netadapter.h"

namespace hbk {
	namespace communication {
		Netadapter::Netadapter()
			: m_name()
			, m_ipv4Addresses()
			, m_ipv6Addresses()
			, m_macAddress()
			, m_fwGuid(0)
			, m_index()
		{
		}

		std::string Netadapter::getIpv4DefaultGateway()
		{
			std::string gatewayString;

			FILE* fp = ::fopen("/proc/net/route", "r");

			if (fp != nullptr) {
				if (fscanf(fp, "%*[^\n]\n") < 0) { // Skip the first line
					::syslog(LOG_ERR, "error reading first line of /proc/net/route!");
				} else {
					while (1) {
						int result;
						char deviceName[64];
						unsigned long destination, gateway, mask;
						unsigned int flags;
						int refCnt, use, metric, mtu, window, irtt;
						constexpr int ROUTE_GW = 0x0002;
						constexpr int ROUTE_UP = 0x0001;

						result = ::fscanf(fp, "%63s%8lx%8lx%8X%8d%8d%8d%8lx%8d%8d%8d\n",
							deviceName, &destination, &gateway, &flags, &refCnt, &use, &metric, &mask,
							&mtu, &window, &irtt);
						if (result != 11) {
							if ((result < 0) && feof(fp)) { /* EOF with no (nonspace) chars read. */
								break;
							}
						}

						if (((flags & ROUTE_GW) != 0) &&  // route is gateway
							((flags & ROUTE_UP) != 0)) { // route is up
								if (destination == INADDR_ANY) { // default gateway
									struct sockaddr_in s_in;
									s_in.sin_addr.s_addr = static_cast < in_addr_t > (gateway);
									gatewayString = ::inet_ntoa(s_in.sin_addr);
									break;
								}
						}
					}
				}
				::fclose(fp);
			}
			return gatewayString;
		}

		bool Netadapter::isFirewireAdapter() const
		{
			if (m_fwGuid) {
				return true;
			} else {
				return false;
			}
		}
	}
}
