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

#include <iostream>
#include <chrono>
#include <future>
#include <thread>
#include <functional>
#include <vector>

#include <gtest/gtest.h>

#include "hbk/sys/eventloop.h"
#include "hbk/sys/timer.h"
#include "hbk/sys/notifier.h"
#include "hbk/sys/executecommand.h"
#include "hbk/exception/exception.hpp"


static int dummyCb()
{
	return 0;
}

static void timerEventHandlerIncrement(bool fired, unsigned int& value, bool& canceled)
{
	if (fired) {
		++value;
		canceled = false;
	}
	else {
		canceled = true;
	}
}


static void executionTimerCb(bool fired, hbk::sys::EventLoop& eventloop)
{
	if (fired) {
		eventloop.stop();
	}
}


static void notifierIncrement(unsigned int& value)
{
	++value;
}



static void decrementCounter(unsigned int &counter, std::promise <void > &notifier)
{
	--counter;
	if (counter==0) {
		notifier.set_value();
	}
}


TEST(eventloop, check_leak)
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
		hbk::sys::Notifier notifier(eventloop);
		executionTimer.set(waitDuration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventloop)));
		notifier.notify();
		eventloop.execute();
	}
	GetProcessHandleCount(GetCurrentProcess(), &fdCountBefore);
#else
	unsigned int fdCountBefore;
	unsigned int fdCountAfter;
	pid_t processId = getpid();
	FILE* pipe;
	std::string cmd;
	// the numbe of file descriptors of this process
	cmd = "ls -1 /proc/" + std::to_string(processId) + "/fd | wc -l";
	pipe = popen(cmd.c_str(), "r");
	char* pResultString = fgets(readBuffer, sizeof(readBuffer), pipe);
	ASSERT_TRUE(pResultString);
	fdCountBefore = static_cast < unsigned int > (std::stoul(readBuffer));
	fclose(pipe);
#endif

	for (unsigned cycle = 0; cycle<10; ++cycle) {
		hbk::sys::EventLoop eventloop;
		hbk::sys::Timer executionTimer(eventloop);
		hbk::sys::Notifier notifier(eventloop);
		executionTimer.set(waitDuration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventloop)));
		notifier.notify();
		eventloop.execute();
	}
	
#ifdef _WIN32
	GetProcessHandleCount(GetCurrentProcess(), &fdCountAfter);
#else
	pipe = popen(cmd.c_str(), "r");
	pResultString = fgets(readBuffer, sizeof(readBuffer), pipe);
	ASSERT_TRUE(pResultString);
	fdCountAfter = static_cast < unsigned int > (std::stoul(readBuffer));
	fclose(pipe);
#endif

	ASSERT_EQ(fdCountBefore, fdCountAfter);
}


/// start the eventloop in a separate thread wait some time and stop it.
TEST(eventloop, stop_test)
{
	hbk::sys::EventLoop eventLoop;

	static const std::chrono::milliseconds waitDuration(300);

	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	std::thread worker(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventLoop)));
	std::this_thread::sleep_for(waitDuration);
	eventLoop.stop();
	worker.join();
	std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();

	std::chrono::milliseconds delta = std::chrono::duration_cast < std::chrono::milliseconds > (endTime-startTime);

	ASSERT_TRUE(delta.count()>=300);
}

/// event loop might be stopped and started again.
TEST(eventloop, eventloop_restart_test)
{
	unsigned int notificationCount = 0;
	hbk::sys::EventLoop eventLoop;
	
	hbk::sys::Notifier notifier(eventLoop);
	hbk::sys::Timer timer(eventLoop);
	static const std::chrono::milliseconds waitDuration(10);
	
	notifier.notify();
	std::this_thread::sleep_for(waitDuration);
	// event loop is not running yet!
	ASSERT_EQ(notificationCount, 0);


	auto timerEventHandlerNop = [](bool)
	{
	};

	timer.set(waitDuration*2, true, timerEventHandlerNop);
	notifier.set(std::bind(&notifierIncrement, std::ref(notificationCount)));

	std::thread worker(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventLoop)));
	std::this_thread::sleep_for(waitDuration);
	// Event loop is running. Notification should happen!
	ASSERT_EQ(notificationCount, 1);
	eventLoop.stop();
	worker.join();


	worker = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventLoop)));
	notifier.notify();
	
	std::this_thread::sleep_for(waitDuration);

	eventLoop.stop();
	worker.join();
	// 2nd notification after restart of event loop.
	ASSERT_EQ(notificationCount, 2);
}


TEST(eventloop, waitforend_test)
{
	hbk::sys::EventLoop eventLoop;
	hbk::sys::Timer executionTimer(eventLoop);
	static const std::chrono::milliseconds duration(100);

	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	executionTimer.set(duration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventLoop)));
	int result = eventLoop.execute();
	std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();

	std::chrono::milliseconds delta = std::chrono::duration_cast < std::chrono::milliseconds > (endTime-startTime);

	ASSERT_EQ(result, 0);
	ASSERT_GE(delta.count(), duration.count()-3);
}

TEST(eventloop, restart_test)
{
	hbk::sys::EventLoop eventLoop;
	hbk::sys::Timer executionTimer(eventLoop);

	static const std::chrono::milliseconds duration(100);

	for (unsigned int i=0; i<10; ++i) {
		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
		executionTimer.set(duration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventLoop)));
		int result = eventLoop.execute();
		std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
		std::chrono::milliseconds delta = std::chrono::duration_cast < std::chrono::milliseconds > (endTime-startTime);
		executionTimer.cancel();

		ASSERT_EQ(result, 0);
		// Under windows we need a delta here. Under linux is the result is exact!
		ASSERT_LE(abs(duration.count() - delta.count()), 10);
	}
}



TEST(eventloop, notify_test)
{
	unsigned int notificationCount = 0;
	//static const std::chrono::milliseconds duration(100);
	hbk::sys::EventLoop eventLoop;
	hbk::sys::Notifier notifier(eventLoop);
	notifier.set(std::bind(&notifierIncrement, std::ref(notificationCount)));
	ASSERT_EQ(notificationCount, 0);

	std::thread worker(std::bind(&hbk::sys::EventLoop::execute, &eventLoop));


	static const unsigned int count = 10;
	for(unsigned int i=0; i<count; ++i) {
		notifier.notify();

#ifdef _WIN32
		/// this is important for windows. linux event_fd accumulates events windows is only able to signal one event.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
	}

	eventLoop.stop();
	worker.join();

	ASSERT_EQ(notificationCount, count);
}

/// recursive notification are to be counted!
TEST(eventloop, recursive_notification_test)
{
	unsigned int notificationCount = 0;
	hbk::sys::EventLoop eventLoop;
	hbk::sys::Notifier notifier(eventLoop);
	
	
	auto recursiveNotifierIncrementCb = [&notificationCount, &notifier]()
	{
		++notificationCount;
		notifier.notify();
	};
	
	notifier.set(recursiveNotifierIncrementCb);
	ASSERT_EQ(notificationCount, 0);

	std::thread worker(std::bind(&hbk::sys::EventLoop::execute, &eventLoop));


	notifier.notify();

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	eventLoop.stop();
	worker.join();

	ASSERT_GT(notificationCount, 1);
}

TEST(eventloop, multiple_event_test)
{
	static const unsigned int NOTIFIER_COUNT = 10;
	unsigned int notificationCount = 0;
	hbk::sys::EventLoop eventLoop;

	std::vector < std::unique_ptr < hbk::sys::Notifier > > notifiers;
	for (unsigned int i=0; i<NOTIFIER_COUNT; ++i) {
		auto notifier = std::make_unique < hbk::sys::Notifier > (eventLoop);
		notifier->set(std::bind(&notifierIncrement, std::ref(notificationCount)));
		notifier->notify();
		notifiers.emplace_back(std::move(notifier));
	}

	ASSERT_EQ(notificationCount, 0);

	std::thread worker(std::bind(&hbk::sys::EventLoop::execute, &eventLoop));

	eventLoop.stop();
	worker.join();

	ASSERT_EQ(notificationCount, NOTIFIER_COUNT);
}

TEST(eventloop, oneshottimer_test)
{
	static const std::chrono::milliseconds timerCycle(100);
	static const std::chrono::milliseconds delta(10);
	static const unsigned int timerCount = 10;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbk::sys::EventLoop eventLoop;

	unsigned int counter = 0;
	bool canceled = false;

	hbk::sys::Timer timer(eventLoop);
	hbk::sys::Timer executionTimer(eventLoop);
	timer.set(timerCycle, false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));
	executionTimer.set(duration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventLoop)));
	int result = eventLoop.execute();
	ASSERT_EQ(counter, 1);
	ASSERT_EQ(result, 0);


	counter = 0;
	std::thread worker = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventLoop)));
	timer.set(timerCycle, false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));

	std::this_thread::sleep_for(timerCycle+delta);
	ASSERT_EQ(counter, 1);

	timer.set(timerCycle, false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));

	std::this_thread::sleep_for(timerCycle+delta);
	ASSERT_EQ(counter, 2);

	eventLoop.stop();
	worker.join();
}

TEST(eventloop, cyclictimer_test)
{
	static const unsigned int excpectedMinimum = 10;
	static const unsigned int timerCycle = 100;
	static const unsigned int timerCount = excpectedMinimum+1;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbk::sys::EventLoop eventLoop;

	unsigned int counter = 0;
	bool canceled = false;

	hbk::sys::Timer cyclicTimer(eventLoop);
	hbk::sys::Timer executionTimer(eventLoop);
	cyclicTimer.set(timerCycle, true, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));
	executionTimer.set(duration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventLoop)));
	int result = eventLoop.execute();

	ASSERT_GE(counter, excpectedMinimum);
	ASSERT_EQ(result, 0);
}

TEST(eventloop, several_timer_events_test)
{
	static const unsigned int timerCycle = 100;
	static const unsigned int timerCount = 10;
	hbk::sys::EventLoop eventLoop;

	unsigned int counter = 0;
	bool canceled = false;

	hbk::sys::Timer cyclicTimer(eventLoop);

	std::thread worker(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventLoop)));

	for (unsigned int i = 0; i < timerCount; ++i) {
		cyclicTimer.set(timerCycle, false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));
		std::this_thread::sleep_for(std::chrono::milliseconds(timerCycle + 10));
	}

	eventLoop.stop();
	worker.join();

	ASSERT_EQ(counter, timerCount);
}


TEST(eventloop, canceltimer_test)
{
	static const unsigned int timerCycle = 100;
	static const unsigned int timerCount = 10;
	static const std::chrono::milliseconds duration(timerCycle * timerCount);
	hbk::sys::EventLoop eventLoop;

	unsigned int counter = 0;
	bool canceled = false;

	hbk::sys::Timer timer(eventLoop);
	hbk::sys::Timer executionTimer(eventLoop);
	timer.set(timerCycle, false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));
	executionTimer.set(duration, false, std::bind(&executionTimerCb, std::placeholders::_1, std::ref(eventLoop)));

	std::thread worker = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventLoop)));

	timer.cancel();

	worker.join();

	ASSERT_EQ(canceled, true);
	ASSERT_EQ(counter, 0);
}

TEST(eventloop, restart_timer_test)
{
	hbk::sys::EventLoop eventLoop;

	static const std::chrono::milliseconds duration(50);

	static const unsigned int restartCount = 10;
	unsigned int counter = 0;
	bool canceled = false;

	std::thread worker(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventLoop)));
	hbk::sys::Timer timer(eventLoop);

	timer.set(std::chrono::milliseconds(duration), false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));
	std::this_thread::sleep_for(duration / 2);
	timer.cancel();
	ASSERT_EQ(canceled, true);

	for (unsigned int i = 0; i < restartCount; ++i) {
		timer.set(std::chrono::milliseconds(duration), false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));
		std::this_thread::sleep_for(duration * 2);
	}
	ASSERT_EQ(canceled, false);
	ASSERT_EQ(counter, restartCount);


	eventLoop.stop();
	worker.join();
}

TEST(eventloop, retrigger_timer_test)
{
	hbk::sys::EventLoop eventLoop;

	static const std::chrono::milliseconds duration(50);

	unsigned int counter = 0;
	bool canceled = false;

	std::thread worker(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventLoop)));
	hbk::sys::Timer timer(eventLoop);

	timer.set(std::chrono::milliseconds(duration), false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));


	// we retrigger the timer before it does signal!
	timer.set(std::chrono::milliseconds(duration * 2), false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));


	std::this_thread::sleep_for(duration);
	// the first trigger would be signaled here. This should not be the case!
	ASSERT_EQ(counter, 0);


	std::this_thread::sleep_for(duration * 2);
	// the second trigger should be signaled here.
	ASSERT_EQ(counter, 1);


	// start timer one more time and make sure event gets signaled
	timer.set(std::chrono::milliseconds(duration / 2), false, std::bind(&timerEventHandlerIncrement, std::placeholders::_1, std::ref(counter), std::ref(canceled)));
	std::this_thread::sleep_for(duration);
	ASSERT_EQ(counter, 2);

	eventLoop.stop();
	worker.join();
}

TEST(eventloop, add_and_remove_event_test)
{
	hbk::sys::EventLoop eventLoop;
#ifdef _WIN32
	static const hbk::sys::event event;
#else
	static const hbk::sys::event event = 1;
#endif
	int result;

	result = eventLoop.addEvent(event, &dummyCb);
	ASSERT_EQ(result, 0);
	// overwriting existing is allowed
	result = eventLoop.addEvent(event, &dummyCb);
	ASSERT_EQ(result, 0);
	result = eventLoop.eraseEvent(event);
	ASSERT_EQ(result, 0);
	// removing non existent should fail
	result = eventLoop.eraseEvent(event);
	ASSERT_EQ(result, -1);

#ifndef _WIN32
	// there was a bug were input event was not removed if input event and output event were set!
	eventLoop.addEvent(1, &dummyCb);
	eventLoop.addOutEvent(1, &dummyCb);
	result = eventLoop.eraseEvent(1);
	ASSERT_EQ(result, 0);
	// removing non existent should fail
	result = eventLoop.eraseEvent(1);
	ASSERT_EQ(result, -1);

	// the same with trying to erase the output event
	eventLoop.addEvent(1, &dummyCb);
	eventLoop.addOutEvent(1, &dummyCb);
	result = eventLoop.eraseOutEvent(1);
	ASSERT_EQ(result, 0);
	// removing non existent should fail
	result = eventLoop.eraseOutEvent(1);
	ASSERT_EQ(result, -1);
#endif
}
TEST(eventloop, add_and_remove_many_events_test)
{
	using Notifiers = std::vector < std::unique_ptr < hbk::sys::Notifier > >;
	static const unsigned int cycleCount = 10;
	static const unsigned int notifierCount = 1000;
	hbk::sys::EventLoop eventLoop;


	Notifiers notifiers;


#ifndef _WIN32
	// under windows, the 1st parameter is a complex parameter
	// invalid function pointer
	int result = eventLoop.addEvent(0, nullptr);
	ASSERT_EQ(result, -1);

	// invalid file descriptor
	result = eventLoop.addEvent(-1, &dummyCb);
	ASSERT_EQ(result, -1);
#endif

	std::thread worker = std::thread(std::bind(&hbk::sys::EventLoop::execute, std::ref(eventLoop)));
	unsigned int counter = notifierCount;
	std::promise < void > promise;
	auto f = promise.get_future();


	for (unsigned int cycle = 0; cycle < cycleCount; ++cycle) {
		for (unsigned int i = 0; i < notifierCount; ++i) {
			auto notifier = std::make_unique < hbk::sys::Notifier > (eventLoop);
			notifier->set(std::bind(&decrementCounter, std::ref(counter), std::ref(promise)));
			notifiers.emplace_back(std::move(notifier));
		}
		for (unsigned int i = 0; i < notifierCount; ++i) {
			notifiers[i]->notify();
		}
		std::future_status status = f.wait_for(std::chrono::milliseconds(100));
		ASSERT_TRUE(status==std::future_status::ready);
		notifiers.clear();
	}

	eventLoop.stop();
	worker.join();
}
