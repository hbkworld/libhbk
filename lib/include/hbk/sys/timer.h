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


#ifndef _HBK__TIMER_H
#define _HBK__TIMER_H

#include <functional>
#include <chrono>

#include "hbk/exception/exception.hpp"
#include "hbk/sys/defines.h"
#include "hbk/sys/eventloop.h"

namespace hbk {
	namespace sys {
		/// A timer running periodically or in single-shot-mode. Starts when setting the period.
		/// Callback routine gets called when period elapsed or running timer gets canceled.

		/// Timers may operate periodically. If timer cycle time elapsed several times until timer is processed, the callback routine is executed only once!
		class Timer {
		public:
			/// called when timer fires or is being cancled
			/// \param false if timer got canceled; true if timer fired
			using Cb_t = std::function < void (bool fired) >;

			/// \throws hbk::exception
			Timer(EventLoop& eventLoop);
			Timer(Timer&& src) = default;

			~Timer();

			/// @param period_ms timer interval in ms
			/// @param repeated set true to trigger timer periodically
			/// @param eventHandler callback function to be called if timer is triggered or canceld
			int set(unsigned int period_ms, bool repeated, Cb_t eventHandler);
			
			/// @param period timer interval in ms
			/// @param repeated set true to trigger timer periodically
			/// @param eventHandler callback function to be called if timer is triggered or canceld
			int set(std::chrono::milliseconds period, bool repeated, Cb_t eventHandler);

			/// if timer is running, callback routine will be called with fired=false
			/// \return 1 success, timer was running; 0 success
			int cancel();

		private:

			/// must not be copied
			Timer(const Timer& op);
			/// must not be assigned
			Timer operator=(const Timer& op);

			/// called by eventloop
			int process();

			event m_fd;
			EventLoop& m_eventLoop;
			Cb_t m_eventHandler;
		};
	}
}
#endif
