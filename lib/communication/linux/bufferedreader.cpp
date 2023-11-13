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

#include <array>
#include <cstring>

#include <sys/socket.h>
#include <sys/uio.h>


#include "hbk/communication/bufferedreader.h"

namespace hbk {
	namespace communication {
		BufferedReader::BufferedReader()
			: m_fillLevel(0)
			, m_alreadyRead(0)
		{
		}

		ssize_t BufferedReader::recv(hbk::sys::event& sockfd, void *buf, size_t desiredLen)
		{
			// check whether there is something left
			size_t bytesLeft = m_fillLevel - m_alreadyRead;

			if(bytesLeft>=desiredLen) {
				// there is more than or as much as desired
				memcpy(buf, m_buffer.data() + m_alreadyRead, desiredLen);
				m_alreadyRead += desiredLen;
				return static_cast < ssize_t > (desiredLen);
			} else if(bytesLeft>0) {
				// return the rest which is less than desired (a short read)
				memcpy(buf, m_buffer.data() + m_alreadyRead, bytesLeft);
				m_alreadyRead = m_fillLevel;
				return static_cast < ssize_t > (bytesLeft);
			}

			// try to read as much as possible into the provided buffer.In addition we fill our internal buffer if there is already more to read.
			// readv saves us from reading into the internal buffer first and copying into the provided buffer afterwards.
			struct iovec iov[2];
			iov[0].iov_base = buf;
			iov[0].iov_len = desiredLen;
			iov[1].iov_base = m_buffer.data();
			iov[1].iov_len = m_buffer.size();

			ssize_t retVal = ::readv(sockfd, iov, 2);
			m_alreadyRead = 0;

			if (retVal>static_cast < ssize_t > (desiredLen)) {
				// readv returns the total number of bytes read
				m_fillLevel = static_cast < size_t > (retVal)-desiredLen;
				return static_cast < ssize_t > (desiredLen);
			}
			m_fillLevel = 0;
			return retVal;
		}
	}
}
