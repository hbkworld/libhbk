#include <iostream>

#include "hbk/communication/netadapterlist.h"
#include "hbk/communication/netlink.h"


int main()
{
	std::cout << "netadapterview" << std::endl;
	hbk::communication::NetadapterList adapterList;
	hbk::communication::NetadapterList::Adapters adapters = adapterList.get();

	for (const auto& iter: adapters) {
		const hbk::communication::Netadapter& adapter = iter.second;
		std::cout << adapter.getName() << std::endl;
		hbk::communication::AddressesWithNetmask ipv4addresses = adapter.getIpv4Addresses();
		for (const hbk::communication::Ipv4Address& ipV4Address: ipv4addresses) {
			std::cout << "\t" << ipV4Address.address << " " << ipV4Address.netmask << std::endl;
		}

		hbk::communication::AddressesWithPrefix ipv6addresses = adapter.getIpv6Addresses();
		for (const hbk::communication::Ipv6Address& ipV6Address: ipv6addresses) {
			std::cout << "\t" << ipV6Address.address << "/" << ipV6Address.prefix << std::endl;
		}
	}
	return EXIT_SUCCESS;
}
