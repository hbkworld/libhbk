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


#ifndef _WMI_H
#define _WMI_H

#ifdef WIN32

#include "hbk/communication/netadapter.h"

struct IWbemLocator;
struct IWbemServices;


namespace hbk {
	namespace communication {

		// This class provides methods to do some basics WMI queries
		class WMI
		{
		public:
			static long initWMI();
			static void uninitWMI();

			static bool isFirewireAdapter(const communication::Netadapter &adapter);
			static long WMI::enableDHCP(const communication::Netadapter &adapter);
			static long WMI::setManualIpV4(const communication::Netadapter &adapter, const communication::Ipv4Address &manualConfig);

		private:
			static IWbemLocator		*m_pLoc;		// WMI locator
			static IWbemServices	*m_pSvc;		// WMI services
		};
	}
}


#endif // WIN32

#endif // _WMI_H
