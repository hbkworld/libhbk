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

#include <WinSock2.h>
#include <IPHlpApi.h>

#include <cstring>

#include "hbk/communication/netlink.h"
#include "hbk/exception/exception.hpp"


namespace hbk {
	namespace communication {
		Netlink::Netlink(communication::NetadapterList &netadapterlist, sys::EventLoop &eventLoop)
			: m_netadapterlist(netadapterlist)
			, m_eventloop(eventLoop)
		{
			m_event.overlapped.hEvent = WSACreateEvent();
		}

		Netlink::~Netlink()
		{
			stop();
		}

		ssize_t Netlink::process()
		{
			m_netadapterlist.update();
			if (m_interfaceAddressEventHandler) {
				m_interfaceAddressEventHandler(COMPLETE, 0, "");
			}
			return orderNextEvent();
		}

		int Netlink::start(interfaceAddressCb_t interfaceAddressEventHandler)
		{
			m_interfaceAddressEventHandler = interfaceAddressEventHandler;
			if (m_interfaceAddressEventHandler) {
				m_interfaceAddressEventHandler(COMPLETE, 0, "");
			}

			orderNextEvent();
			m_eventloop.addEvent(m_event, std::bind(&Netlink::process, std::ref(*this)));

			return 0;
		}

		int Netlink::stop()
		{
			m_eventloop.eraseEvent(m_event);
			return CloseHandle(m_event.overlapped.hEvent);
		}
		
		int Netlink::orderNextEvent()
		{
			if (m_event.overlapped.Internal == STATUS_PENDING) {
				return 0;
			}

			DWORD ret = NotifyAddrChange(&m_event.fileHandle, &m_event.overlapped);
			if (ret != WSA_IO_PENDING) {
				return -1;
			}
			return 0;
		}
	}
}
