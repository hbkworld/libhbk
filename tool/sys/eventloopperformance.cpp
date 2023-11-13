#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>

#include "hbk/sys/eventloop.h"
#include "hbk/sys/notifier.h"


static const size_t EVENTLIMIT = 100000;

static void notify()
{
	hbk::sys::EventLoop eventloop;
	
	hbk::sys::Notifier notifier(eventloop);
	size_t eventCount = 0;
	auto notifierCb = [&eventloop, &notifier, &eventCount]()
	{
		eventCount++;
		if (eventCount>=EVENTLIMIT) {
			eventloop.stop();
		} else {
			notifier.notify();
		}
	};
	
	notifier.set(notifierCb);
	notifier.notify();
	std::chrono::high_resolution_clock::time_point t1;
	std::chrono::high_resolution_clock::time_point t2;

	t1 = std::chrono::high_resolution_clock::now();
	eventloop.execute();

	t2 = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds diff = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

	std::cout << "execution time for " << EVENTLIMIT << " event notifications: " << diff.count() << "µs" << std::endl;
}

static void addRemove()
{
	hbk::sys::EventLoop eventloop;
	std::chrono::high_resolution_clock::time_point t1;
	std::chrono::high_resolution_clock::time_point t2;
	t1 = std::chrono::high_resolution_clock::now();
	for (size_t count = 0; count<EVENTLIMIT; ++count) {
		// By creating the notifier it is added to the eventloop. It is removed on destruction.
		hbk::sys::Notifier tempNotifier(eventloop);
	}
	t2 = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds diff = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
	std::cout << "execution time for create/destruct " << EVENTLIMIT << " notifiers: " << diff.count() << "µs" << std::endl;
}

int main()
{
	notify();
	addRemove();
	return EXIT_SUCCESS;
}
