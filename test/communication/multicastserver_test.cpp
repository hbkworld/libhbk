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

#include <chrono>
#include <functional>
#include <future>
#include <iostream>

#include <gtest/gtest.h>

#include "hbk/communication/multicastserver.h"
#include "hbk/communication/netadapter.h"
#include "hbk/communication/netadapterlist.h"

#include "hbk/sys/eventloop.h"
#include "hbk/sys/timer.h"


static int receiveAndKeep(hbk::communication::MulticastServer& mcs, std::promise < std::string >& received)
{
	ssize_t result;
	do {
		char buf[1024];
		unsigned int adapterIndex;
		int ttl;
		result = mcs.receiveTelegram(buf, sizeof(buf), adapterIndex, ttl);
		if (result>0) {
			// we may receive the same message several times because there might be several interfaces used for sending
			std::string receivedData(buf, static_cast< size_t >(result));
			received.set_value(receivedData);
			std::cout << __FUNCTION__ << " '" << receivedData << "'" <<std::endl;
		}
	} while(result>=0);
	return 0;
}

static int receiveAndDiscard(hbk::communication::MulticastServer& mcs)
{
	ssize_t result;
	char buf[1024];
	unsigned int adapterIndex;
	int ttl;
	do {
		result = mcs.receiveTelegram(buf, sizeof(buf), adapterIndex, ttl);
	} while(result>=0);
	return 0;
}

static void executionTimerCb(bool fired, hbk::sys::EventLoop& eventloop)
{
	if (fired) {
		eventloop.stop();
	}
}

TEST(communication, check_leak)
{
	static const std::chrono::milliseconds waitDuration(1);

	char readBuffer[1024] = "";

	
	hbk::communication::NetadapterList adapterlist;

#ifdef _WIN32
	DWORD fdCountBefore;
	DWORD fdCountAfter;


	{
		// Do one create and destruct cycle under windows before retrieving the number of handles before. This is important beacuse a lot of handles will be created on the first run.
		hbk::sys::EventLoop eventloop;
		hbk::sys::Timer executionTimer(eventloop);
		hbk::communication::MulticastServer mcs(adapterlist, eventloop);
		executionTimer.set(waitDuration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventloop)));
		eventloop.execute();

		mcs.stop();
	}
	GetProcessHandleCount(GetCurrentProcess(), &fdCountBefore);

#else
	unsigned int fdCountBefore;
	unsigned int fdCountAfter;

	FILE* pipe;
	std::string cmd;
	pid_t processId = getpid();

	// the numbe of file descriptors of this process
	cmd = "ls -1 /proc/" + std::to_string(processId) + "/fd | wc -l";
	pipe = popen(cmd.c_str(), "r");
	char* pResultString = fgets(readBuffer, sizeof(readBuffer), pipe);
	ASSERT_TRUE(pResultString);
	fdCountBefore = std::stoul(readBuffer);
	fclose(pipe);
#endif


	for (unsigned cycle = 0; cycle<10; ++cycle) {
		hbk::sys::EventLoop eventloop;
		hbk::sys::Timer executionTimer(eventloop);
		hbk::communication::MulticastServer mcs(adapterlist, eventloop);
		executionTimer.set(waitDuration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventloop)));
		eventloop.execute();

		mcs.stop();
	}
	
#ifdef _WIN32
	GetProcessHandleCount(GetCurrentProcess(), &fdCountAfter);
#else
	pipe = popen(cmd.c_str(), "r");
	pResultString = fgets(readBuffer, sizeof(readBuffer), pipe);
	ASSERT_TRUE(pResultString);
	fdCountAfter = std::stoul(readBuffer);
	fclose(pipe);
#endif

	ASSERT_EQ(fdCountBefore, fdCountAfter);
}

TEST(communication, start_send_stop_test)
{
	static const char MULTICASTGROUP[] = "239.255.77.177";
	static const unsigned int UDP_PORT = 22222;
	static const unsigned int CYCLECOUNT = 100;

	static const std::string MSG = "test1test2";
	int result;


	hbk::sys::EventLoop eventloop;
	std::future < int > workerResult = std::async(std::launch::async, std::bind(&hbk::sys::EventLoop::execute, std::ref(eventloop)));
	hbk::communication::NetadapterList adapters;
	hbk::communication::MulticastServer mcsReceiver(adapters, eventloop);
	hbk::communication::MulticastServer mcsSender(adapters, eventloop);
	hbk::communication::Netadapter firstadapter = adapters.get().cbegin()->second;

	result = mcsSender.start(MULTICASTGROUP, UDP_PORT, std::bind(&receiveAndDiscard, std::placeholders::_1));
	ASSERT_EQ(result,0);

	mcsSender.setMulticastLoop(true);
	for (unsigned int i=0; i<CYCLECOUNT; ++i) {
		std::promise < std::string > received;
		std::future < std::string > f = received.get_future();

		result = mcsReceiver.start(MULTICASTGROUP, UDP_PORT, std::bind(&receiveAndKeep, std::placeholders::_1, std::ref(received)));
		ASSERT_EQ(result, 0);
		mcsReceiver.addAllInterfaces();
		mcsSender.send(MSG.c_str(), MSG.length());

		std::future_status fStatus = f.wait_for(std::chrono::milliseconds(1000));
		ASSERT_TRUE(fStatus == std::future_status::ready);
		mcsReceiver.stop();
		ASSERT_EQ(MSG, f.get());

		std::cout << __FUNCTION__ << " " << i << std::endl;
	}

	mcsSender.stop();

	eventloop.stop();
	workerResult.wait();

	std::cout << __FUNCTION__ << " done" << std::endl;
}

TEST(communication, test_join)
{
	static const char MULTICASTGROUP[] = "239.255.77.177";
	static const unsigned int UDP_PORT = 22222;

	hbk::sys::EventLoop eventloop;
	hbk::communication::NetadapterList adapters;
	hbk::communication::MulticastServer server(adapters, eventloop);
	int result = server.start(MULTICASTGROUP, UDP_PORT, std::bind(&receiveAndDiscard, std::placeholders::_1));
	ASSERT_TRUE(result == 0);

	result = server.addInterface("bla");
	ASSERT_TRUE(result < 0);

	result = server.addInterface("127.0.0.1");
	ASSERT_TRUE(result >= 0);

	server.stop();
}
