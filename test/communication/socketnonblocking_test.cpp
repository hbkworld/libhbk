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

#include <string>
#include <functional>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include "hbk/communication/socketnonblocking.h"
#include "hbk/communication/tcpserver.h"
#include "socketnonblocking_test.h"
#include "hbk/sys/eventloop.h"
#include "hbk/sys/timer.h"


//#define ENABLE_IPV6

static const char server[] = "127.0.0.1";


namespace hbk {
	namespace communication {
		namespace test {

			static const unsigned int PORT = 22222;

			static void executionTimerCb(bool fired, hbk::sys::EventLoop& eventloop)
			{
				if (fired) {
					eventloop.stop();
				}
			}

			serverFixture::serverFixture()
				: m_server(m_eventloop)
			{
				int result = m_server.start(PORT, 3, std::bind(&serverFixture::acceptCb, this, std::placeholders::_1));
				if (result) {
					throw(std::runtime_error("could not start tcp server"));
				}
			}

			serverFixture::~serverFixture()
			{
				stop();
			}

			size_t serverFixture::getClientCount() const
			{
				return m_workers.size();
			}
			
			void serverFixture::start()
			{
				m_serverWorker = std::async(std::launch::async, std::bind(&hbk::sys::EventLoop::execute, std::ref(m_eventloop)));
			}

			void serverFixture::stop()
			{
				m_eventloop.stop();
				try {
					m_serverWorker.wait();
				} catch (...) {
				}

			}


			void serverFixture::acceptCb(clientSocket_t worker)
			{
				static int clientId = 0;
				++clientId;
				m_workers[clientId] = std::move(worker);
				m_workers[clientId]->setDataCb(std::bind(&serverFixture::serverEcho, this, clientId));
			}

			ssize_t serverFixture::serverEcho(int clientId)
			{
				char buffer[1024];
				ssize_t result;

				do {
					result = m_workers[clientId]->receive(buffer, sizeof(buffer));
					if (result>0) {
						result = m_workers[clientId]->sendBlock(buffer, static_cast < size_t >(result), false);
					} else if (result==0) {
						// socket got closed
						m_workers.erase(clientId);
					} else {
#ifdef _WIN32
						int retVal = WSAGetLastError();
						if ((retVal != WSAEWOULDBLOCK) && (retVal != ERROR_IO_PENDING) && (retVal != WSAEINPROGRESS)) {
#else
						if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
#endif


							// a real error
							m_workers.erase(clientId);
						}
					}
				} while (result>0);
				return result;
			}



			int serverFixture::clientReceive(hbk::communication::SocketNonblocking& socket)
			{
				char buffer[1024];
				ssize_t result;

				result = socket.receive(buffer, sizeof(buffer));
				if (result>0) {
					// trailing '\0' won't be added
					m_answer += buffer;
				}

				return static_cast < int > (result);
			}

			int serverFixture::clientReceiveTarget(hbk::communication::SocketNonblocking& socket, std::string& target)
			{
				char buffer[1024];
				ssize_t result;

				result = socket.receive(buffer, sizeof(buffer));
				if (result>0) {
					// trailing '\0' won't be added
					target += buffer;
				}

				return static_cast < int >(result);
			}
			

			int serverFixture::clientReceiveSingleBytes(hbk::communication::SocketNonblocking& socket)
			{
				char buffer;
				ssize_t result;

				result = socket.receive(&buffer, sizeof(buffer));
				if (result > 0) {
					if (buffer != '\0') {
						m_answer += buffer;
					}
				}

				return static_cast < int >(result);
			}

		TEST(communication, check_tcpserver)
		{
			int result;
			hbk::sys::EventLoop eventloop;
			hbk::communication::TcpServer tcpServer(eventloop);

			auto acceptCb = [](clientSocket_t clientSocket)
			{

			};

			// \todo CI runs with root rights!
			// ports below 1024 are not allowed for normal users
			//result = tcpServer.start(1024-1, 5, acceptCb);
			//ASSERT_EQ(result, -1);

			// no proper callback function
			result = tcpServer.start(1024-1, 5, hbk::communication::TcpServer::Cb_t());
			ASSERT_EQ(result, -1);
		}

		TEST(Communication, check_construct)
		{
			hbk::sys::EventLoop eventloop;
			int invalidFd = -1;
			EXPECT_THROW(hbk::communication::SocketNonblocking socket(invalidFd, eventloop), std::runtime_error);

		}

		TEST(communication, check_leak)
		{
			static const std::chrono::milliseconds waitDuration(1);
		
			char readBuffer[1024] = "";
		
			
#ifdef _WIN32
			DWORD fdCountBefore;
			DWORD fdCountAfter;

			{
				// Do one create and destruct cycle under windows before retrieving the number of handles before. This is important beacuse a lot of handles will be created on the first run.
				hbk::sys::EventLoop eventloop;
				hbk::sys::Timer executionTimer(eventloop);
				hbk::communication::SocketNonblocking socket(eventloop);
				executionTimer.set(waitDuration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventloop)));
				eventloop.execute();
			}
			GetProcessHandleCount(GetCurrentProcess(), &fdCountBefore);
#else
			unsigned long fdCountBefore;
			unsigned long fdCountAfter;
			
			FILE* pipe;
			std::string cmd;
			pid_t processId = getpid();

			// the numbe of file descriptors of this process
			cmd = "ls -1 /proc/" + std::to_string(processId) + "/fd | wc -l";
			pipe = popen(cmd.c_str(), "r");
			if(fgets(readBuffer, sizeof(readBuffer), pipe)==nullptr) {
				FAIL() << "Could not get number of fds of process";
			}
			fdCountBefore = std::stoul(readBuffer);
			fclose(pipe);
#endif


			for (unsigned cycle = 0; cycle<10; ++cycle) {
				hbk::sys::EventLoop eventloop;
				hbk::sys::Timer executionTimer(eventloop);
				hbk::communication::SocketNonblocking socket(eventloop);
				executionTimer.set(waitDuration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventloop)));
				eventloop.execute();
			}
			
#ifdef _WIN32
			GetProcessHandleCount( GetCurrentProcess(), &fdCountAfter);
#else
			pipe = popen(cmd.c_str(), "r");
			if(fgets(readBuffer, sizeof(readBuffer), pipe)==nullptr) {
				FAIL() << "Could not get number of fds of process";
			}
			fdCountAfter = std::stoul(readBuffer);
			fclose(pipe);
#endif
		
			ASSERT_EQ(fdCountBefore, fdCountAfter);
		}
		
		TEST_F(serverFixture, connect_test)
		{
			int result;
			start();
			hbk::communication::SocketNonblocking client(m_eventloop);
			for (unsigned int cycleIndex = 0; cycleIndex < 1000; ++cycleIndex) {
				result = client.connect(server, std::to_string(PORT));
				ASSERT_TRUE(result == 0) << strerror(errno);
				client.disconnect();
			}

			// wrong port! Should fail
			result = client.connect(server, std::to_string(PORT + 1));
			ASSERT_TRUE(result == -1) <<  strerror(errno);
		}

		TEST_F(serverFixture, connect_multiple_test)
		{
			static const unsigned int CLIENT_COUNT = 32;
			static const unsigned int CYCLE_COUNT = 100;

			int result;
			start();

			std::vector < std::unique_ptr < hbk::communication::SocketNonblocking > > clients;

			for (size_t clientIndex = 0; clientIndex < CLIENT_COUNT; ++clientIndex) {
				clients.emplace_back(std::make_unique < hbk::communication::SocketNonblocking >(m_eventloop));
			}

			for (unsigned int cycleIndex = 0; cycleIndex < CYCLE_COUNT; ++cycleIndex) {
				for (unsigned int clientIndex = 0; clientIndex < CLIENT_COUNT; ++clientIndex) {
					result = clients[clientIndex]->connect(server, std::to_string(PORT));
					ASSERT_TRUE(result == 0) << strerror(errno);

					clients[clientIndex]->disconnect();
				}
			}
		}

		TEST_F(serverFixture, send_recv_test)
		{
			ssize_t result;
			static const char msg[] = "hallo";
			char response[1024];

			start();

			hbk::communication::SocketNonblocking client(m_eventloop);
			result = client.connect(server, std::to_string(PORT));
			ASSERT_TRUE(result == 0) << strerror(errno);

			result = client.send(msg, sizeof(msg), false);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			ASSERT_TRUE(result == sizeof(msg)) << strerror(errno);
			result = client.receive(response, sizeof(msg));
			ASSERT_TRUE(result == sizeof(msg)) << strerror(errno);
			ASSERT_TRUE(strcmp(response, msg)==0);

			result = client.send(msg, sizeof(msg), false);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			ASSERT_TRUE(result == sizeof(msg)) << strerror(errno);
			for( unsigned int cycleIndex=0; cycleIndex<sizeof(msg); ++cycleIndex) {
				result = client.receive(response, 1);
				ASSERT_TRUE(result == 1) << strerror(errno);
				ASSERT_EQ(response[0], msg[cycleIndex]);
			}
			result = client.receive(response, 1);
			ASSERT_EQ(result, -1);

			client.disconnect();

		}

		TEST_F(serverFixture, sendblock_recvblock_test)
			{
				ssize_t result;
				static const std::string msg = "hallo";
				char response[1024];

				start();

				hbk::communication::SocketNonblocking client(m_eventloop);
				//client.setDataCb(std::bind(&serverFixture::clientReceiveSingleBytes, this, std::placeholders::_1));
				result = client.connect(server, std::to_string(PORT));
				ASSERT_TRUE(result == 0) << strerror(errno);

				for( unsigned int cycleIndex=0; cycleIndex<100; ++cycleIndex) {
					result = client.sendBlock(msg.c_str(), msg.length(), false);
					ASSERT_TRUE(static_cast <size_t >(result) == msg.length()) << strerror(errno);
					result = client.receiveComplete(response, msg.length(), 100);
					ASSERT_TRUE(static_cast <size_t >(result) == msg.length()) << strerror(errno);
					response[result] = '\0';
					ASSERT_EQ(response, msg);
				}

				result = client.sendBlock(msg.c_str(), msg.length(), false);
				ASSERT_TRUE(static_cast <size_t >(result) == msg.length()) << strerror(errno);
				for( unsigned int cycleIndex=0; cycleIndex<msg.length(); ++cycleIndex) {
					result = client.receiveComplete(response, 1, 100);
					ASSERT_TRUE(result == 1) << strerror(errno);
					ASSERT_EQ(response[0], msg[cycleIndex]);
				}
				result = client.receiveComplete(response, 1, 100);
				ASSERT_EQ(result, -1);


				// force timeout!
				result = client.receiveComplete(response, msg.length(), 100);
				ASSERT_EQ(result, -1);

				// nothing to read and no time to wait
				result = client.receiveComplete(response, msg.length(), 0);
				ASSERT_EQ(result, -1);

				client.disconnect();


				stop();
			}
			

		TEST_F(serverFixture, echo_test)
		{
			ssize_t result;
			static const char msg[] = "hallo";
			static const char msg2[] = "!";

			start();

			hbk::communication::SocketNonblocking client(m_eventloop);
			client.setDataCb(std::bind(&serverFixture::clientReceiveSingleBytes, this, std::placeholders::_1));
			result = client.connect(server, std::to_string(PORT));
			ASSERT_TRUE(result == 0) << strerror(errno);

			clearAnswer();
			result = client.sendBlock(msg, sizeof(msg), false);
			ASSERT_TRUE(result == sizeof(msg)) << strerror(errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			std::string answer = getAnswer();
			ASSERT_EQ(answer, msg);

			clearAnswer();
			result = client.sendBlock(msg2, sizeof(msg2), false);
			ASSERT_TRUE(result == sizeof(msg2)) << strerror(errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			answer = getAnswer();
			ASSERT_EQ(answer, msg2);


			clearAnswer();
			client.sendBlock(msg, sizeof(msg), false);
			client.sendBlock(msg2, sizeof(msg2), false);

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			answer = getAnswer();
			ASSERT_EQ(answer, std::string(msg) + msg2);

			client.disconnect();



			stop();
		}

		TEST_F(serverFixture, multiclient_echo_test)
			{
				static const size_t clientCount = 10;
				ssize_t result;
				static const std::string msgPrefix = "hallo";

				start();

				std::vector < std::unique_ptr < hbk::communication::SocketNonblocking > > clients;
				
				for (size_t clientIndex=0; clientIndex<clientCount; ++clientIndex) {
					clients.emplace_back( std::make_unique < hbk::communication::SocketNonblocking> (m_eventloop));
				}
				
				unsigned int index = 0;
				for (auto &iter: clients) {
					std::string msg = msgPrefix + std::to_string(index++);
					result = iter->connect(server, std::to_string(PORT));
					ASSERT_TRUE(result == 0) << strerror(errno);
					iter->setDataCb(std::bind(&serverFixture::clientReceiveSingleBytes, this, std::placeholders::_1));
					
					clearAnswer();
					result = iter->sendBlock(msg.c_str(), msg.length(), false);
					ASSERT_TRUE(static_cast < size_t > (result) == msg.length()) << strerror(errno);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					std::string answer = getAnswer();
					ASSERT_EQ(msg, answer);
				}

				for (auto &iter: clients) {
					iter->disconnect();
				}


				stop();
			}
			
			
#ifdef ENABLE_IPV6
		// under ci no support for ipv6
		TEST_F(serverFixture, echo_test_ipv6)
		{
			ssize_t result;
			static const char msg[] = "hallo";

			start();

			hbk::communication::SocketNonblocking client(m_eventloop);
			result = client.connect(server, std::to_string(PORT));
			ASSERT_TRUE(result == 0) << strerror(errno);
			client.setDataCb(std::bind(&serverFixture::clientReceiveSingleBytes, this, std::placeholders::_1));

			clearAnswer();
			result = client.sendBlock(msg, sizeof(msg), false);
			ASSERT_TRUE(result == sizeof(msg)) << strerror(errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			std::string answer = getAnswer();
			ASSERT_EQ(answer, msg);

			client.disconnect();


			stop();
		}
#endif

		TEST_F(serverFixture, setting_data_callback)
		{
			ssize_t result;
			static const char msg[] = "hallo";

			start();

			hbk::communication::SocketNonblocking client(m_eventloop);
			result = client.connect(server, std::to_string(PORT));
			ASSERT_TRUE(result == 0) << strerror(errno);
			client.setDataCb(std::bind(&serverFixture::clientReceiveSingleBytes, this, std::placeholders::_1));

			clearAnswer();
			result = client.sendBlock(msg, sizeof(msg), false);
			ASSERT_TRUE(result == sizeof(msg)) << strerror(errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			std::string answer = getAnswer();
			ASSERT_EQ(answer, msg);

			// set callback function again
			client.setDataCb(std::bind(&serverFixture::clientReceive, this, std::placeholders::_1));
			clearAnswer();
			result = client.sendBlock(msg, sizeof(msg), false);

			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			ASSERT_TRUE(result == sizeof(msg)) << strerror(errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			answer = getAnswer();
			clearAnswer();
			ASSERT_EQ(answer, msg);

			client.disconnect();



			stop();
		}

		TEST_F(serverFixture, writev_test_list)
		{
			ssize_t result;
			static const size_t bufferSize = 100000;
			static const size_t blockCount = 10;
			static const size_t blockSize = bufferSize/blockCount;
			char buffer[bufferSize] = "a";
			uint8_t smallBuffer[] = {"hallo"};

			hbk::communication::dataBlocks_t dataBlocks;

			for(unsigned int i=0; i<blockCount; ++i) {
				hbk::communication::dataBlock_t dataBlock(&buffer[i*blockSize], blockSize);
				dataBlocks.push_back(dataBlock);
			}

			start();

			hbk::communication::SocketNonblocking client(m_eventloop);
			result = client.connect(server, std::to_string(PORT));
			ASSERT_TRUE(result == 0) << strerror(errno);

			client.setDataCb(std::bind(&serverFixture::clientReceive, this, std::placeholders::_1));

			clearAnswer();
			result = client.sendBlocks(dataBlocks);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ASSERT_TRUE(result == bufferSize) << strerror(errno);

			clearAnswer();

			hbk::communication::dataBlock_t smallDatablocks[4];
			size_t bufferCount = sizeof(smallDatablocks)/sizeof(hbk::communication::dataBlock_t);

			for (size_t blockIndex = 0; blockIndex<bufferCount; ++blockIndex) {
				smallDatablocks[blockIndex].pData = smallBuffer;
				smallDatablocks[blockIndex].size = sizeof(smallBuffer);
			}


			result = client.sendBlocks(smallDatablocks, bufferCount, true);
			ASSERT_TRUE(static_cast<size_t>(result) == sizeof(smallBuffer)*bufferCount) << strerror(errno);
			result = client.sendBlocks(smallDatablocks, bufferCount, false);
			ASSERT_TRUE(static_cast<size_t>(result) == sizeof(smallBuffer)*bufferCount) << strerror(errno);

			client.disconnect();

			stop();
		}


#define BLOCKCOUNT 10

		TEST_F(serverFixture, writev_test_array)
			{
				int result;
				static const size_t bufferSize = 100000;
				static const size_t blockSize = bufferSize/BLOCKCOUNT;
				char buffer[bufferSize] = "a";

				hbk::communication::dataBlock_t dataBlockArray[BLOCKCOUNT];
				hbk::communication::dataBlocks_t dataBlocks;

				for(unsigned int i=0; i<BLOCKCOUNT; ++i) {
					dataBlockArray[i].pData = &buffer[i*blockSize];
					dataBlockArray[i].size = blockSize;
				}

				start();

				hbk::communication::SocketNonblocking client(m_eventloop);
				result = client.connect(server, std::to_string(PORT));
				ASSERT_TRUE(result == 0) << strerror(errno);

				client.setDataCb(std::bind(&serverFixture::clientReceive, this, std::placeholders::_1));

				clearAnswer();
				result = static_cast < int > (client.sendBlocks(dataBlockArray, 10));
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				client.disconnect();

				ASSERT_TRUE(result == bufferSize) << strerror(errno);


				stop();
			}

		TEST_F(serverFixture, receive_wouldblock)
		
			{
				char buffer[1000];
				hbk::communication::SocketNonblocking client(m_eventloop);
				client.connect(server, std::to_string(PORT));

				ssize_t result = client.receive(buffer, sizeof(buffer));
				ASSERT_EQ(result, -1);
#ifndef _WIN32
				ASSERT_EQ(errno, EWOULDBLOCK);
#else
				ASSERT_EQ(WSAGetLastError(), WSA_IO_PENDING);
#endif
			}
			
#ifndef _WIN32
			TEST_F(serverFixture, send_wouldblock)
			{
				size_t bytesSend = 0;
				unsigned int sendBufferSize = 1000;
				socklen_t len;
				char data[1000000] = { 0 };
				hbk::communication::SocketNonblocking client(m_eventloop);
				client.connect(server, std::to_string(PORT));
				unsigned int notifiyCount = 0;

				auto lambda = [&notifiyCount](hbk::communication::SocketNonblocking&)
				{
					++notifiyCount;
					return 0;
				};

				client.setOutDataCb(lambda);
				// callback function gets called once to check for pending work...
				ASSERT_EQ(notifiyCount, 1);
				
				int result = setsockopt(client.getEvent(), SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(sendBufferSize));
				ASSERT_EQ(result, 0);
				len = sizeof(sendBufferSize);
				getsockopt(client.getEvent(), SOL_SOCKET, SO_SNDBUF, &sendBufferSize, &len);
				
				static const unsigned int CYCLECOUNT = 10;
				for (unsigned int cycleIndex=0; cycleIndex<CYCLECOUNT; ++cycleIndex) {
					notifiyCount = 0;
					ssize_t sendResult;
					// send until send buffer is full
					do {
						sendResult = client.send(data, sizeof(data), false);
						bytesSend += static_cast < size_t > (sendResult);
					} while (sendResult>0);
					ASSERT_GT(bytesSend, sendBufferSize);
					ASSERT_EQ(errno, EWOULDBLOCK);
					
					// server is not receiving, hence socket won't become writable and callback function is not being called.
					ASSERT_EQ(notifiyCount, 0);
					start();
					// server is receiving. As a result socket becomes wriable again. Callback function is called!
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					
					ASSERT_EQ(notifiyCount, 1);
					
					// We should be able to send again
					sendResult = client.send(data, sizeof(data), false);
					ASSERT_GT(sendResult, 0);
					stop();
				}
			}
#endif
		}
	}
}
