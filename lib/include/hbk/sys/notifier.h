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

#ifndef _HBK__SYS_NOTIFIER_H
#define _HBK__SYS_NOTIFIER_H

#include "hbk/sys/defines.h"
#include "hbk/exception/exception.hpp"

namespace hbk {
	namespace sys {
		class EventLoop;

		/// Notify someone else waiting for a specific event
		/// If notified n (>0) times until notifier is processed, the callback routine is executed n times!
		class Notifier {
			using Cb_t = std::function < void () >;
		public:
			/// \throws hbk::exception
			Notifier(EventLoop& eventLoop);
			Notifier(Notifier&& src);

			virtual ~Notifier();

			/// register a callback function for notifications
			/// @param eventHandler Callback function to be executed upon notification
			int set(Cb_t eventHandler);

			/// Eventhandler gets called in event loop context
			int notify();
		private:
			/// must not be copied
			Notifier(const Notifier& op);
			/// must not be assigned
			Notifier operator=(const Notifier& op);

			/// called by eventloop
			int process();

			event m_fd;
			EventLoop& m_eventLoop;
			Cb_t m_eventHandler;
		};
	}
}
#endif
