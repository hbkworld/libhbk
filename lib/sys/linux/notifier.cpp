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

#include <cstring>

#include <sys/eventfd.h>
#include <unistd.h>

#include "hbk/exception/exception.hpp"
#include "hbk/sys/notifier.h"
#include "hbk/sys/eventloop.h"

// default callback does nothing and returns 0 to tell the eventloop that there is nothing more to do
static int nop()
{
	return 0;
}

namespace hbk {
	namespace sys {
		Notifier::Notifier(EventLoop& eventLoop)
			: m_fd(eventfd(0, EFD_NONBLOCK))
			, m_eventLoop(eventLoop)
			, m_eventHandler(&nop)
		{
			if (m_fd<0) {
				throw hbk::exception::exception("could not create event fd");
			}
			
			if (m_eventLoop.addEvent(m_fd, std::bind(&Notifier::process, this))<0) {
				throw hbk::exception::exception("could not add notifier to event loop");
			}
		}

		Notifier::Notifier(Notifier &&src)
			: m_fd(src.m_fd)
			, m_eventLoop(src.m_eventLoop)
			, m_eventHandler(src.m_eventHandler)
		{
			// reroute epoll event handling!
			if (m_eventLoop.addEvent(m_fd, std::bind(&Notifier::process, this))<0) {
				throw hbk::exception::exception("could not add notifier to event loop");
			}
			src.m_fd = -1;
		}

		Notifier::~Notifier()
		{
			m_eventLoop.eraseEvent(m_fd);
			close(m_fd);
		}

		int Notifier::set(Cb_t eventHandler)
		{
			if (eventHandler) {
				m_eventHandler = eventHandler;
			} else {
				m_eventHandler = &nop;
			}
			return 0;
		}


		int Notifier::notify()
		{
			static const uint64_t value = 1;
			return static_cast < int > (write(m_fd, &value, sizeof(value)));
		}

		int Notifier::process()
		{
			uint64_t eventCount = 0;
			// it is sufficient to read once in order to rearm
			ssize_t result = ::read(m_fd, &eventCount, sizeof(eventCount));
			if (static_cast < size_t > (result)!=sizeof(eventCount)) {
				return -1;
			}
			for (uint64_t i=0; i<eventCount; i++) {
				m_eventHandler();
			}
			return 0;
		}
	}
}
