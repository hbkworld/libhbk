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


#include "hbk/communication/ipv6address.h"

TEST(ipv6address, calc_ipv6_link_local)
{
	std::string macAddress = "11:22:33:44:55:66";
	std::string ipv6LinkLocalAddress = hbk::communication::Ipv6Address::linkLocalFromMac(macAddress);
	ASSERT_EQ(ipv6LinkLocalAddress, "fe80::1322:33ff:fe44:5566");
	
	std::string incompleteAddress = "22:33:44:55:66";
	EXPECT_ANY_THROW(hbk::communication::Ipv6Address::linkLocalFromMac(incompleteAddress));
}

TEST(ipv6address, check_address_type)
{
	bool result;
	result = hbk::communication::Ipv6Address::isLinkLocalAddress("fe80::40ab:a528:ada6:3da5");
	ASSERT_EQ(result, true);
	result = hbk::communication::Ipv6Address::isLinkLocalAddress("FE80::40ab:a528:ada6:3da5");
	ASSERT_EQ(result, true);
	result = hbk::communication::Ipv6Address::isLinkLocalAddress("fe0::40ab:a528:ada6:3da5");
	ASSERT_EQ(result, false);
	result = hbk::communication::Ipv6Address::isLinkLocalAddress("bla");
	ASSERT_EQ(result, false);
}

TEST(ipv6address, check_equal)
{
	hbk::communication::Ipv6Address address1;
	hbk::communication::Ipv6Address address2;
	
	address1.address = "2003:d4:bf21:f000:6359:d9fb:66ce:2bd1";
	address1.prefix = 64;
	
	ASSERT_FALSE(address1.equal(address2));
	address2 = address1;
	ASSERT_TRUE(address1.equal(address2));
	
	// check case insentive
	std::transform(address2.address.begin(), address2.address.end(), address2.address.begin(), 
								 [](unsigned char c){ return std::toupper(c); }
								);
	ASSERT_TRUE(address1.equal(address2));
	
	// check leading zeroes
	address1.address = "2003:0d4:bf21:f000:6359:d9fb:66ce:2bd1";
	ASSERT_TRUE(address1.equal(address2));
}

TEST(ipv6address, getIpv4MappedIpv6Address)
{
	std::string ipv6Address;
	std::string ipv4Address;

	ipv6Address = "::ffff:192.0.2.128";
	ipv4Address = hbk::communication::Ipv6Address::getIpv4MappedAddress(ipv6Address);
	ASSERT_TRUE(ipv4Address.length()>0);

	// wrong prefix
	ipv6Address = "::fff:192.0.2.128";
	ipv4Address = hbk::communication::Ipv6Address::getIpv4MappedAddress(ipv6Address);
	ASSERT_TRUE(ipv4Address.length()==0);

	// invalid ipv4 addresses
	ipv6Address = "::ffff:192.0.128";
	ipv4Address = hbk::communication::Ipv6Address::getIpv4MappedAddress(ipv6Address);
	ASSERT_TRUE(ipv4Address.length()==0);
	ipv6Address = "::ffff:300.1.0.128";
	ipv4Address = hbk::communication::Ipv6Address::getIpv4MappedAddress(ipv6Address);
	ASSERT_TRUE(ipv4Address.length()==0);
	ipv6Address = "::ffff:192.0.2.";
	ipv4Address = hbk::communication::Ipv6Address::getIpv4MappedAddress(ipv6Address);
	ASSERT_TRUE(ipv4Address.length()==0);
}
