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

#include <algorithm>

#include <gtest/gtest.h>


#include "hbk/communication/ipv4address.h"



TEST(ipv4address, check_valid_ipaddresses_test)
{
	bool result;
	result = hbk::communication::Ipv4Address::isValidManualAddress("172.19.2.4");
	ASSERT_EQ(result, true);
	result = hbk::communication::Ipv4Address::isValidManualAddress("172.169.254.0");
	ASSERT_EQ(result, true);
}

TEST(communication, check_invalid_ipaddresses_test)
{
	bool result;
	result = hbk::communication::Ipv4Address::isValidManualAddress("172.19.2");
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidManualAddress("172.19.2.");
	ASSERT_EQ(result, false);
}

TEST(ipv4address, check_invalid_netmask_test)
{
	bool result;
	result = hbk::communication::Ipv4Address::isValidNetmask("kaputt");
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidNetmask("300.1.2.3");
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidNetmask("0.0.0.0");
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidNetmask("255.255.255.255");
	ASSERT_EQ(result, false);

	// valide!
	ASSERT_TRUE(hbk::communication::Ipv4Address::isValidNetmask("255.255.255.0"));
}

TEST(ipv4address, check_address_type)
{
	bool result;
	result = hbk::communication::Ipv4Address::isApipaAddress("169.254.2.6");
	ASSERT_EQ(result, true);
	result = hbk::communication::Ipv4Address::isApipaAddress("169.254.2");
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isApipaAddress("169.254.2.");
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isApipaAddress("10.169.254.6");
	ASSERT_EQ(result, false);
}

TEST(ipv4address, check_prefix_from_netmask)
{
	int prefix;

	prefix = hbk::communication::Ipv4Address::getPrefixFromNetmask("128.0.0.0");
	ASSERT_EQ(prefix, 1);
	prefix = hbk::communication::Ipv4Address::getPrefixFromNetmask("255.0.0.0");
	ASSERT_EQ(prefix, 8);
	prefix = hbk::communication::Ipv4Address::getPrefixFromNetmask("255.255.0.0");
	ASSERT_EQ(prefix, 16);
	prefix = hbk::communication::Ipv4Address::getPrefixFromNetmask("255.255.255.0");
	ASSERT_EQ(prefix, 24);
	prefix = hbk::communication::Ipv4Address::getPrefixFromNetmask("255.255.255.255");
	ASSERT_EQ(prefix, 32);
	prefix = hbk::communication::Ipv4Address::getPrefixFromNetmask("bla");
	ASSERT_EQ(prefix, -1);
	prefix = hbk::communication::Ipv4Address::getPrefixFromNetmask("255.255.255.");
	ASSERT_EQ(prefix, -1);
	// we do not allow gaps!
	prefix = hbk::communication::Ipv4Address::getPrefixFromNetmask("127.0.0.0");
	ASSERT_EQ(prefix, -1);
	prefix = hbk::communication::Ipv4Address::getPrefixFromNetmask("64.0.0.0");
	ASSERT_EQ(prefix, -1);
}

TEST(ipv4address, check_netmask_from_prefix)
{
	std::string netmask;
	netmask = hbk::communication::Ipv4Address::getNetmaskFromPrefix(8);
	ASSERT_EQ(netmask, "255.0.0.0");
	netmask = hbk::communication::Ipv4Address::getNetmaskFromPrefix(16);
	ASSERT_EQ(netmask, "255.255.0.0");
	netmask = hbk::communication::Ipv4Address::getNetmaskFromPrefix(32);
	ASSERT_EQ(netmask, "255.255.255.255");
	netmask = hbk::communication::Ipv4Address::getNetmaskFromPrefix(33);
	ASSERT_EQ(netmask, "");
}

TEST(ipv4address, check_forbidden_ipaddresses_test)
{
	bool result;
	result = hbk::communication::Ipv4Address::isValidManualAddress("not an address");
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidManualAddress("0.1.2.3"); // uppermost byte must be set
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidManualAddress("127.0.0.1"); // loopback
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidManualAddress("169.254.0.1"); // APIPA
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidManualAddress("224.4.7.1"); // multicast
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidManualAddress("254.4.7.1"); // experimental
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv4Address::isValidManualAddress("0.0.0.0"); // non-routable meta-address
	ASSERT_EQ(result, true);
}

TEST(ipv4address, check_equal)
{
	hbk::communication::Ipv4Address address1;
	hbk::communication::Ipv4Address address2;
	
	address1.address = "1.2.3.4";
	address1.netmask = "255.255.0.0";
	
	ASSERT_FALSE(address1.equal(address2));
	address2 = address1;
	ASSERT_TRUE(address1.equal(address2));
	
}

TEST(ipv4address, check_subnet)
{
	hbk::communication::Ipv4Address address;
	std::string subnet;
	address.address = "172.19.1.2";
	address.netmask = "255.255.0.0";
	subnet = address.getSubnet();
	ASSERT_EQ(subnet, "172.19.0.0");

	address.address = "172.130.1.2";
	address.netmask = "255.128.0.0";
	subnet = address.getSubnet();
	ASSERT_EQ(subnet, "172.128.0.0");

	address.address = "172.130.1.2";
	address.netmask = "255.128.0.2";
	subnet = address.getSubnet();
	ASSERT_EQ(subnet, "172.128.0.2");
}
