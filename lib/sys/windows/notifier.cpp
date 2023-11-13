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

#include <iostream>
#include <cstring>

#include <WinSock2.h>

#include "hbk/sys/notifier.h"
#include "hbk/sys/eventloop.h"

namespace hbk {
	namespace sys {
		Notifier::Notifier(EventLoop& eventLoop)
			: m_fd()
			, m_eventLoop(eventLoop)
			, m_eventHandler()
		{
			m_fd.completionPort = m_eventLoop.getCompletionPort();
			m_fd.overlapped.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			m_eventLoop.addEvent(m_fd, std::bind(&Notifier::process, std::ref(*this)));
		}

		Notifier::~Notifier()
		{
			m_eventLoop.eraseEvent(m_fd);
			CloseHandle(m_fd.overlapped.hEvent);
		}


		int Notifier::notify()
		{
			if (PostQueuedCompletionStatus(m_fd.completionPort, 0, (ULONG_PTR)m_fd.overlapped.hEvent, &m_fd.overlapped)) {
				return 0;
			} else {
				return -1;
			}
		}

		int Notifier::process()
		{
			if (m_eventHandler) {
				m_eventHandler();
			}
			return 0;
		}

		int Notifier::set(Cb_t eventHandler)
		{
			m_eventHandler = eventHandler;
			return 0;
		}
	}
}
