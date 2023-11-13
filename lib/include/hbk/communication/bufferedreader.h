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


#ifndef _HBK__BUFFEREDREADER_H
#define _HBK__BUFFEREDREADER_H

#include "hbk/sys/defines.h"
#ifdef _WIN32
#ifndef ssize_t
#define ssize_t int
#endif
#else
#include <sys/types.h>
#endif

#include <array>

namespace hbk {
	namespace communication {
		/// Try to receive a big chunk even if only a small amount of data is requested.
		/// This reduces the number of system calls being made.
		/// Return the requested amount of data and keep the remaining data.
		/// \warning not reentrant
		class BufferedReader
		{
		public:
			BufferedReader();

			BufferedReader(BufferedReader&& op) = default;
			BufferedReader& operator=(BufferedReader&& op) = default;

			/// Not to be copied since we don't want to interfere with each other.
			BufferedReader(const BufferedReader& op) = delete;
			/// Not to be copied since we don't want to interfere with each other.
			BufferedReader& operator=(const BufferedReader& op) = delete;

			/// behaves like standard recv
			ssize_t recv(hbk::sys::event& ev, void *buf, size_t len);

		private:
			// std::array is movable
			std::array < unsigned char, 65536> m_buffer;
			size_t m_fillLevel;
			size_t m_alreadyRead;
		};
	}
}
#endif // BUFFEREDREADER_H
