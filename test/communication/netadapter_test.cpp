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

#include <gtest/gtest.h>


#include "hbk/communication/netadapter.h"
#include "hbk/communication/netadapterlist.h"

#include "hbk/string/split.h"
#include "hbk/sys/executecommand.h"


TEST(communication, get_adapter_by_x)
{
	hbk::communication::NetadapterList::Adapters adapters = hbk::communication::NetadapterList().get();
	if (adapters.empty()) {
		return;
	}
	
	std::string adapterName = adapters.cbegin()->second.getName();
	hbk::communication::Netadapter adapter = hbk::communication::NetadapterList().getAdapterByName(adapterName);
	ASSERT_EQ(adapterName, adapter.getName());
	
	unsigned int adapterIndex = adapters.cbegin()->second.getIndex();
	adapter = hbk::communication::NetadapterList().getAdapterByInterfaceIndex(adapterIndex);
	ASSERT_EQ(adapterName, adapter.getName());
	
	// those have to fail
	ASSERT_THROW(hbk::communication::NetadapterList().getAdapterByName(adapterName + "invalid"), std::runtime_error);
	ASSERT_THROW(hbk::communication::NetadapterList().getAdapterByInterfaceIndex(adapterIndex+1000), std::runtime_error);
}

TEST(communication, check_mac_address)
{
	hbk::communication::NetadapterList::Adapters adapters = hbk::communication::NetadapterList().get();
	if (adapters.empty()) {
		return;
	}
	
	std::string macAddress = adapters.cbegin()->second.getMacAddressString();
	ASSERT_GT(macAddress.length(), 0);
	hbk::string::tokens tokens = hbk::string::split(macAddress, ':');
	ASSERT_EQ(tokens.size(), 6);
}

TEST(communication, check_occupied_subnet)
{
	// get first ipv4 address of first interface to provoke conflict.
	std::string occupyingInterfaceName;
	std::string interfaceName;
	hbk::communication::NetadapterList netadapterList;
	hbk::communication::AddressesWithNetmask addresses;

	// get the first active adapter.
	hbk::communication::Netadapter adapter;
	hbk::communication::NetadapterList::Adapters adapters = netadapterList.get();
	for( hbk::communication::NetadapterList::Adapters::const_iterator iter=adapters.cbegin(); iter!=adapters.cend(); ++iter) {
		adapter = iter->second;
		addresses = adapter.getIpv4Addresses();
		if (addresses.empty()==false) {
			break;
		}
	}

	if (addresses.empty()) {
		EXPECT_TRUE(false) << "No interface with ipv4 address available. Test can not be performed!";
		return;
	}

	hbk::communication::NetadapterList::AdapterArray adapterArray = netadapterList.getArray();
	ASSERT_EQ(adapters.cbegin()->second.getIndex(), adapterArray[0].getIndex());

	hbk::communication::Ipv4Address firstAddress = *addresses.cbegin();
	interfaceName = adapter.getName();
	
	occupyingInterfaceName = netadapterList.checkSubnet("", firstAddress);
	ASSERT_EQ(occupyingInterfaceName, interfaceName);


	// check with exclusion of this interface
	occupyingInterfaceName = netadapterList.checkSubnet(interfaceName, firstAddress);
	ASSERT_EQ(occupyingInterfaceName, "");

	
	// localhost is not contained in netadapterlist, hence nobody does occupy "127.0.0.1"
	hbk::communication::Ipv4Address localHostAddress;
	localHostAddress.address = "127.0.0.1";
	localHostAddress.netmask = "255.0.0.0";
	
	occupyingInterfaceName = netadapterList.checkSubnet(interfaceName, localHostAddress);
	ASSERT_EQ(occupyingInterfaceName, "");
}
