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

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <string>

#include <WinSock2.h>
#undef max
#undef min

#include <cstring>

#include "hbk/communication/bufferedreader.h"

static WSABUF signalBuffer = { 0, nullptr };

namespace hbk {
	namespace communication {
		BufferedReader::BufferedReader()
			: m_fillLevel(0)
			, m_alreadyRead(0)
		{
		}

		ssize_t BufferedReader::recv(hbk::sys::event& ev, void *buf, size_t desiredLen)
		{
			// check whether there is something left
			size_t bytesLeft = m_fillLevel - m_alreadyRead;
			if (bytesLeft >= desiredLen) {
				memcpy(buf, m_buffer + m_alreadyRead, desiredLen);
				m_alreadyRead += desiredLen;
				return static_cast <ssize_t> (desiredLen);
			}
			else if (bytesLeft > 0) {
				// return the rest which is less than desired (a short read)
				memcpy(buf, m_buffer + m_alreadyRead, bytesLeft);
				m_alreadyRead = m_fillLevel;
				return static_cast <ssize_t> (bytesLeft);
			}

			if (ev.overlapped.Internal == STATUS_PENDING) {
				// in this case we are already waiting for data
				WSASetLastError(WSA_IO_PENDING);
				return -1;
			}

			WSABUF buffers[2];
			DWORD Flags = 0;
			DWORD numberOfBytesRecvd;
			buffers[0].buf = reinterpret_cast <CHAR*> (buf);
			buffers[0].len = static_cast <ULONG> (desiredLen);
			buffers[1].buf = reinterpret_cast <CHAR*> (m_buffer);
			buffers[1].len = sizeof(m_buffer);

			int retVal = WSARecv(reinterpret_cast <SOCKET> (ev.fileHandle), buffers, 2, &numberOfBytesRecvd, &Flags, nullptr, nullptr);
			m_alreadyRead = 0;
			if (retVal < 0) {
				if (WSAGetLastError() == WSAEWOULDBLOCK) {
					// important: Makes io completion to be signalled by the next arriving byte
					Flags = 0;
					if (WSARecv(reinterpret_cast <SOCKET> (ev.fileHandle), &signalBuffer, 1, nullptr, &Flags, &ev.overlapped, nullptr) == 0) {
						// workaround: if the operation completed with success, there was something to be received. In this case the event will not be signaled. Call WSARecv once more to get the data.
						Flags = 0;
						retVal = WSARecv(reinterpret_cast <SOCKET> (ev.fileHandle), buffers, 2, &numberOfBytesRecvd, &Flags, nullptr, nullptr);
						if (numberOfBytesRecvd>static_cast < DWORD > (desiredLen)) {
							// WSARecv returns the total number of bytes read
							m_fillLevel = numberOfBytesRecvd - desiredLen;
							return static_cast < ssize_t > (desiredLen);
						}
						m_fillLevel = 0;
						return numberOfBytesRecvd;
					}
				}

				m_fillLevel = 0;
				return retVal;
			}

			if (numberOfBytesRecvd>static_cast < DWORD > (desiredLen)) {
				// WSARecv returns the total number of bytes read
				m_fillLevel = numberOfBytesRecvd - desiredLen;
				return static_cast < ssize_t > (desiredLen);
			}
			m_fillLevel = 0;
			return numberOfBytesRecvd;
		}
	}
}
