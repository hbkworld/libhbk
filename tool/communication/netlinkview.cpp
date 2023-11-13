#include <cstdlib>
#include <functional>
#include <iostream>

#include "hbk/sys/eventloop.h"
#include "hbk/communication/netadapterlist.h"
#include "hbk/communication/netlink.h"

static hbk::communication::NetadapterList adapters;

static void netlinkCb(hbk::communication::Netlink::event_t event, unsigned int adapterIndex, const std::string& ipv4Address)
{
	static unsigned int eventCount = 0;
	std::string adapterName;
	try {
		adapterName = adapters.getAdapterByInterfaceIndex(adapterIndex).getName();
	} catch(const std::runtime_error&) {
		// to be ignored because adapter does not exist anymore
	}

	++eventCount;
	std::cout << eventCount << ": ";
	switch (event) {
		case hbk::communication::Netlink::ADDRESS_ADDED:
			// not supported under Windows
			std::cout << "new interface address appeared (adapter " << adapterIndex << " (" << adapterName << "), ipv4 address=" << ipv4Address << ")" << std::endl;
			break;
		case hbk::communication::Netlink::ADDRESS_REMOVED:
			// not supported under Windows
			std::cout << "interface address disappeared (adapter " << adapterIndex << " (" << adapterName << "), ipv4 address=" << ipv4Address << ")" << std::endl;
			break;
		case hbk::communication::Netlink::COMPLETE:
			std::cout << "complete reconfiguration" << std::endl;
			break;

		case hbk::communication::Netlink::LINK_ADDED:
			// not supported under Windows
			std::cout << "interface " << adapterIndex << " (" << adapterName << ") got up" << std::endl;
			break;

		case hbk::communication::Netlink::LINK_REMOVED:
			// not supported under Windows
			std::cout << "interface " << adapterIndex << " (" << adapterName << ") went down" << std::endl;
			break;
	}
}

int main()
{
	hbk::sys::EventLoop eventloop;
	hbk::communication::Netlink netlink(adapters, eventloop);
	
	netlink.start(std::bind(&netlinkCb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	eventloop.execute();
	return EXIT_SUCCESS;
}
