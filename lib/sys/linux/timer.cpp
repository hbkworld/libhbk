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

#include <syslog.h>
#include <cstring>
#include <inttypes.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring> // for memset()


#include "hbk/exception/exception.hpp"

#include "hbk/sys/timer.h"

// default callback does nothing and returns 0 to tell the eventloop that there is nothing more to do
static int nop(bool)
{
	return 0;
}

namespace hbk {
	namespace sys {
		Timer::Timer(EventLoop &eventLoop)
			: m_fd(timerfd_create(CLOCK_BOOTTIME, TFD_NONBLOCK))
			, m_eventLoop(eventLoop)
			, m_eventHandler(&nop)
		{
			if (m_fd<0) {
				throw hbk::exception::exception(std::string("could not create timer fd '") + strerror(errno) + "'");
			}
			if (m_eventLoop.addEvent(m_fd, std::bind(&Timer::process, this))<0) {
				throw hbk::exception::exception("could not add timer to event loop");
			}
		}

		Timer::~Timer()
		{
			m_eventLoop.eraseEvent(m_fd);
			close(m_fd);
		}

		int Timer::set(std::chrono::milliseconds period, bool repeated, Cb_t eventHandler)
		{
			return set(static_cast < unsigned int > (period.count()), repeated, eventHandler);
		}

		int Timer::set(unsigned int period_ms, bool repeated, Cb_t eventHandler)
		{
			if (period_ms==0) {
				return -1;
			}

			struct itimerspec timespec;
			memset (&timespec, 0, sizeof(timespec));
			unsigned int period_s = period_ms / 1000;
			unsigned int rest = period_ms % 1000;

			timespec.it_value.tv_sec = period_s;
			timespec.it_value.tv_nsec = rest * 1000 * 1000;
			if (repeated) {
				timespec.it_interval.tv_sec = period_s;
				timespec.it_interval.tv_nsec = rest * 1000 * 1000;
			}
			if (m_eventHandler) {
				m_eventHandler = eventHandler;
			} else {
				m_eventHandler = &nop;
			}
			return timerfd_settime(m_fd, 0, &timespec, nullptr);
		}

		int Timer::cancel()
		{
			int retval = 0;
			struct itimerspec timespec;

			// Before calling callback function with fired=false, we need to clear the callback routine. Otherwise a recursive call might happen
			Cb_t originalEventHandler = m_eventHandler;
			m_eventHandler = &nop;

			if (timerfd_gettime(m_fd, &timespec)==-1) {
				syslog(LOG_ERR, "error getting remaining time of timer %d '%s'", m_fd, strerror(errno));
				return -1;
			}
			if ( (timespec.it_value.tv_sec != 0) || (timespec.it_value.tv_nsec != 0) ) {
				// timer is running
				if (originalEventHandler) {
					originalEventHandler(false);
				}
				retval = 1;
			}

			memset (&timespec, 0, sizeof(timespec));
			timerfd_settime(m_fd, 0, &timespec, nullptr);

			return retval;
		}

		int Timer::process()
		{
			// We do not really need to know the count. Read gives 8 byte if timer triggered at least once.
			uint64_t timerEventCount = 0;
			// it is sufficient to read once in order to rearm cyclic timers
			ssize_t result = ::read(m_fd, &timerEventCount, sizeof(timerEventCount));
			if (static_cast < size_t > (result)==sizeof(timerEventCount)) {
				m_eventHandler(true);
			}
			return 0;
		}
	}
}
