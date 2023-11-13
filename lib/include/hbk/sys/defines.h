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


#ifndef _HBK__SYS_DEFINES_H
#define _HBK__SYS_DEFINES_H

#ifdef _WIN32
#include <WinSock2.h>
#ifndef ssize_t
#define ssize_t int
#endif
#else
#include <unistd.h>
#endif

#include <functional>

namespace hbk {
	namespace sys {

#ifdef _WIN32
		struct event {
			event()
				: completionPort(INVALID_HANDLE_VALUE)
				, fileHandle(INVALID_HANDLE_VALUE)
			{
				memset(&overlapped, 0, sizeof(overlapped));
			}
			/// the io completion port used by the event loop
			HANDLE completionPort;
			OVERLAPPED overlapped;
			HANDLE fileHandle;
		};
#else
		typedef int event;
#endif
		typedef std::function < int () > EventHandler_t;
	}
}
#endif
