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

#ifndef _EventLoop_H
#define _EventLoop_H


#include <unordered_map>
#ifdef _WIN32
	#include <WinSock2.h>
	#include <Windows.h>
#else
	#include <sys/epoll.h>
#endif
#include <functional>
#include <mutex>

#include "hbk/exception/exception.hpp"
#include "hbk/sys/defines.h"

namespace hbk {
	namespace sys {
		/// The event loop is not responsible for handling errors returned by any callback routine. Error handling is to be done by the callback routine itself.
		class EventLoop {
		public:
			/// \throws hbk::exception
			EventLoop();

			EventLoop(EventLoop& el) = delete;
			EventLoop operator=(EventLoop& el) = delete;
			EventLoop(const EventLoop& el) = delete;
			EventLoop operator=(const EventLoop& el) = delete;

			virtual ~EventLoop();

			/// existing event handler of an fd will be replaced
			/// \param fd a non-blocking file descriptor to observe
			/// \param eventHandler callback function to be called if file descriptor gets readable.
			int addEvent(event fd, const EventHandler_t &eventHandler);


#ifndef _WIN32
			/// existing event handler of an fd will be replaced
			/// \param fd a non-blocking file descriptor to observe
			/// \param eventHandler callback function to be called if file descriptor gets writable.
			int addOutEvent(event fd, const EventHandler_t &eventHandler);
#endif

			/// remove an event from the event loop
			int eraseEvent(event fd);
#ifndef _WIN32
			/// remove an event from the event loop
			int eraseOutEvent(event fd);
#endif

			/// \return 0 stopped; -1 error
			int execute();

			/// Execution of the event loop is stopped. Events won't be handled afterwards!
			void stop();

#ifdef _WIN32
			HANDLE getCompletionPort() const
			{
				return m_completionPort;
			}
#endif

		private:
#ifdef _WIN32
			typedef std::unordered_map <HANDLE, EventHandler_t > eventInfos_t;
			HANDLE m_completionPort;
			HANDLE m_hEventLog;
#else
			/// maximum number of events that may be queued with epoll_wait()
			static const unsigned int MAXEVENTS = 16;
			/// events from epoll_wait
			struct epoll_event m_events[MAXEVENTS];
			/// number of events from epoll_wait
			int m_eventCount;

			struct EventsHandlers_t {
				/// callback function for events for reading
				EventHandler_t inEvent;
				/// callback function for events for writing
				EventHandler_t outEvent;
			};
			using eventInfos_t = std::unordered_map <event, EventsHandlers_t >;

			int m_epollfd;
			event m_stopFd;
#endif
			/// protects access on events structures
			std::recursive_mutex m_eventInfosMtx;
			eventInfos_t m_eventInfos;
		};
	}
}
#endif
