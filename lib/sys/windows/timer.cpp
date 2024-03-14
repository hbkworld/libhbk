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
#include <WinSock2.h>

#include <cstring>
#include <stdint.h>


#include "hbk/sys/timer.h"


static void CALLBACK timerCb(void *pData, BOOLEAN fired)
{
	if (fired) {
		hbk::sys::event* pEvent = reinterpret_cast <hbk::sys::event*> (pData);
		PostQueuedCompletionStatus(pEvent->completionPort, 0, (ULONG_PTR)pEvent->overlapped.hEvent, &pEvent->overlapped);
	}
}


namespace hbk {
	namespace sys {
		Timer::Timer(EventLoop& eventLoop)
			: m_fd()
			, m_eventLoop(eventLoop)
			, m_eventHandler()
		{
			m_fd.completionPort = m_eventLoop.getCompletionPort();
		}

		Timer::Timer(Timer &&src)
			: m_fd(src.m_fd)
			, m_eventLoop(src.m_eventLoop)
			, m_eventHandler(src.m_eventHandler)
		{
			src.m_fd.completionPort = INVALID_HANDLE_VALUE
		}

		Timer::~Timer()
		{
			if (m_fd.completionPort == INVALID_HANDLE_VALUE) {
				return;
			}
			m_eventLoop.eraseEvent(m_fd);
			if (m_fd.overlapped.hEvent) {
				DeleteTimerQueueTimer(nullptr, m_fd.overlapped.hEvent, nullptr);
			}
		}

		int Timer::set(std::chrono::milliseconds period, bool repeated, Cb_t eventHandler)
		{
			return set(static_cast <unsigned int>(period.count()), repeated, eventHandler);
		}

		int Timer::set(unsigned int period_ms, bool repeated, Cb_t eventHandler)
		{
			m_eventLoop.eraseEvent(m_fd);

			if (m_fd.overlapped.hEvent) {
				DeleteTimerQueueTimer(nullptr, m_fd.overlapped.hEvent, nullptr);
				m_fd.overlapped.hEvent = nullptr;
			}

			DWORD repeatPeriod;
			if (repeated) {
				repeatPeriod = period_ms;
			}
			else {
				repeatPeriod = 0;
			}

			m_eventHandler = eventHandler;

			if (CreateTimerQueueTimer(&m_fd.overlapped.hEvent, nullptr, &timerCb, &m_fd, period_ms, repeatPeriod, WT_EXECUTEINTIMERTHREAD)) {
				return m_eventLoop.addEvent(m_fd, std::bind(&Timer::process, std::ref(*this)));
			} else {
				return -1;
			}
		}

		int Timer::process()
		{
			if (m_eventHandler) {
				m_eventHandler(true);
			}
			return 0;
		}


		int Timer::cancel()
		{
			m_eventLoop.eraseEvent(m_fd);

			if (m_fd.overlapped.hEvent) {
				DeleteTimerQueueTimer(nullptr, m_fd.overlapped.hEvent, nullptr);
				m_fd.overlapped.hEvent = nullptr;
			}
			if (m_eventHandler) {
				m_eventHandler(false);
			}
			return 0;
		}
	}
}
