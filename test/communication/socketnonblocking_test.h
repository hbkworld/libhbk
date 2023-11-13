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

#ifndef __HBK__COMMUNICATION_SOCKETTEST_H
#define __HBK__COMMUNICATION_SOCKETTEST_H

#include <future>
#include <map>

#include <gtest/gtest.h>

#include "hbk/communication/socketnonblocking.h"
#include "hbk/communication/tcpserver.h"

#include "hbk/sys/eventloop.h"


namespace hbk {
	namespace communication {
		namespace test {

			class serverFixture: public ::testing::Test  {
			public:
				int clientReceive(SocketNonblocking &socket);
				int clientReceiveTarget(SocketNonblocking& socket, std::string& target);
				int clientReceiveSingleBytes(SocketNonblocking& socket);

				int clientNotify(unsigned int& count);

				void start();
				void stop();
				size_t getClientCount() const;
				sys::EventLoop m_eventloop;
			protected:
				serverFixture();
				virtual ~serverFixture();
				void acceptCb(clientSocket_t worker);
				ssize_t serverEcho(int clientId);
				void clearAnswer()
				{
					m_answer.clear();
				}

				std::string getAnswer() const
				{
					return m_answer;
				}

			private:
				std::future < int > m_serverWorker;

				std::map < int, clientSocket_t> m_workers;
				TcpServer m_server;

				std::string m_answer;
			};
		}
	}
}

#endif
