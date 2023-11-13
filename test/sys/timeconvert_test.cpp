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
#include <ctime>
#include <stdint.h>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif

#include <gtest/gtest.h>

#include "hbk/sys/timeconvert.h"



TEST(test_timeutil, ntp_fraction_conversion)
{
	uint32_t calculatedNtpFraction;
	uint32_t expectedNtpFraction;
	uint32_t calculatedNanoseconds;
	uint32_t calculatedMicroseconds;
	uint32_t expectedNanoseconds;
	hbk::sys::TimeConvert::TimeConvert::NtpStruct calculatedNtpTime;

	expectedNanoseconds = 6250;
	calculatedNtpFraction = hbk::sys::TimeConvert::nanoseconds2Ntpfraction(expectedNanoseconds);
	calculatedNanoseconds = hbk::sys::TimeConvert::ntpfraction2nanoseconds(calculatedNtpFraction);
	ASSERT_EQ(expectedNanoseconds, calculatedNanoseconds);
	expectedNanoseconds = 25000;
	calculatedNtpFraction = hbk::sys::TimeConvert::nanoseconds2Ntpfraction(expectedNanoseconds);
	calculatedNanoseconds = hbk::sys::TimeConvert::ntpfraction2nanoseconds(calculatedNtpFraction);
	ASSERT_EQ(expectedNanoseconds, calculatedNanoseconds);

	expectedNanoseconds = 25;
	calculatedNtpFraction = hbk::sys::TimeConvert::nanoseconds2Ntpfraction(expectedNanoseconds);
	calculatedNanoseconds = hbk::sys::TimeConvert::ntpfraction2nanoseconds(calculatedNtpFraction);
	ASSERT_EQ(expectedNanoseconds, calculatedNanoseconds);

	expectedNanoseconds = 2;
	calculatedNtpFraction = hbk::sys::TimeConvert::nanoseconds2Ntpfraction(expectedNanoseconds);
	calculatedNanoseconds = hbk::sys::TimeConvert::ntpfraction2nanoseconds(calculatedNtpFraction);
	ASSERT_EQ(expectedNanoseconds, calculatedNanoseconds);

	expectedNanoseconds = 1;
	calculatedNtpFraction = hbk::sys::TimeConvert::nanoseconds2Ntpfraction(expectedNanoseconds);
	calculatedNanoseconds = hbk::sys::TimeConvert::ntpfraction2nanoseconds(calculatedNtpFraction);
	ASSERT_EQ(expectedNanoseconds, calculatedNanoseconds);

	expectedNanoseconds = 2000;
	calculatedNtpFraction = hbk::sys::TimeConvert::nanoseconds2Ntpfraction(expectedNanoseconds);
	calculatedNanoseconds = hbk::sys::TimeConvert::ntpfraction2nanoseconds(calculatedNtpFraction);
	ASSERT_EQ(expectedNanoseconds, calculatedNanoseconds);

	expectedNanoseconds = 10000;
	calculatedNtpFraction = hbk::sys::TimeConvert::nanoseconds2Ntpfraction(expectedNanoseconds);
	calculatedNanoseconds = hbk::sys::TimeConvert::ntpfraction2nanoseconds(calculatedNtpFraction);
	calculatedMicroseconds = hbk::sys::TimeConvert::ntpfraction2microseconds(calculatedNtpFraction);
	ASSERT_EQ(expectedNanoseconds, calculatedNanoseconds);
	ASSERT_EQ(expectedNanoseconds, calculatedMicroseconds*1000);

	calculatedNtpFraction = hbk::sys::TimeConvert::microseconds2Ntpfraction(500000);
	expectedNtpFraction = 1u << 31;
	ASSERT_EQ(calculatedNtpFraction, expectedNtpFraction);
	expectedNanoseconds = 500000000;
	calculatedNtpFraction = hbk::sys::TimeConvert::nanoseconds2Ntpfraction(expectedNanoseconds);
	calculatedNtpTime = hbk::sys::TimeConvert::nanoseconds2NtpStruct(expectedNanoseconds);
	expectedNtpFraction = 1u << 31;
	ASSERT_EQ(calculatedNtpFraction, expectedNtpFraction);
	ASSERT_EQ(calculatedNtpTime.fractions, calculatedNtpFraction);

	calculatedNtpFraction = hbk::sys::TimeConvert::microseconds2Ntpfraction(250000);
	expectedNtpFraction = 1u << 30;
	ASSERT_EQ(calculatedNtpFraction, expectedNtpFraction);
	calculatedNtpFraction = hbk::sys::TimeConvert::nanoseconds2Ntpfraction(250000000);
	expectedNtpFraction = 1u << 30;
	ASSERT_EQ(calculatedNtpFraction, expectedNtpFraction);
}

TEST(test_timeutil,  epoch_test)
{
	uint64_t ntp;
	double doubleNtpEpoch;
	double doubleValue;

	// std::chrono uses the unix epoch that is 1.1.1970
	std::chrono::time_point < std::chrono::system_clock > unixEpoch;
	std::time_t epoch_time = std::chrono::system_clock::to_time_t(unixEpoch);
	ASSERT_EQ(epoch_time, 0);
	
	// ntp has the epoch 1.1.1900
	doubleNtpEpoch = 0;
	ntp = hbk::sys::TimeConvert::unixtime2Ntp(doubleNtpEpoch);
	uint32_t seconds = ntp >> 32;
	ASSERT_EQ(seconds, hbk::sys::TimeConvert::ntpSecondOffset);
	doubleValue = hbk::sys::TimeConvert::ntp2double(ntp);
	ASSERT_EQ(doubleValue, doubleNtpEpoch);
}


/// when converting from double unix time to ntp and back, we need to consider the offset between the epochs!
TEST(test_timeutil, unixtime_as_double_2Ntp_test)
{
	uint64_t ntp;
	uint64_t ntpExpected;
	double doubleValueRequest;
	double doubleValue;

	doubleValueRequest = 0.5;
	ntp = hbk::sys::TimeConvert::unixtime2Ntp(doubleValueRequest);
	ntp = hbk::sys::TimeConvert::unixtime2Ntp(doubleValueRequest);
	ntpExpected = (static_cast < uint64_t > (hbk::sys::TimeConvert::ntpSecondOffset) << 32) + 0x80000000;
	ASSERT_EQ(ntp, ntpExpected);
	doubleValue = hbk::sys::TimeConvert::ntp2double(ntp);
	ASSERT_EQ(doubleValue, doubleValueRequest);

	doubleValueRequest = 0.25;
	ntp = hbk::sys::TimeConvert::unixtime2Ntp(doubleValueRequest);
	doubleValue = hbk::sys::TimeConvert::ntp2double(ntp);
	ASSERT_EQ(doubleValue, doubleValueRequest);

	doubleValueRequest = 0.125;
	ntp = hbk::sys::TimeConvert::unixtime2Ntp(doubleValueRequest);
	doubleValue = hbk::sys::TimeConvert::ntp2double(ntp);
	ASSERT_EQ(doubleValue, doubleValueRequest);

	doubleValueRequest = 0.0;
	ntp = hbk::sys::TimeConvert::unixtime2Ntp(doubleValueRequest);
	ntpExpected = (static_cast < uint64_t > (hbk::sys::TimeConvert::ntpSecondOffset) << 32);
	ASSERT_EQ(ntp, ntpExpected);
	doubleValue = hbk::sys::TimeConvert::ntp2double(ntp);
	ASSERT_EQ(doubleValue, doubleValueRequest);

	doubleValueRequest = 1000000.0;
	ntp = hbk::sys::TimeConvert::unixtime2Ntp(doubleValueRequest);
	ntpExpected = (static_cast < uint64_t > (hbk::sys::TimeConvert::ntpSecondOffset+1000000) << 32);
	ASSERT_EQ(ntp, ntpExpected);
	doubleValue = hbk::sys::TimeConvert::ntp2double(ntp);
	ASSERT_EQ(doubleValue, doubleValueRequest);
}

TEST(test_timeutil, unixtime_as_timeval_2Ntp_test)
{
	timeval tv;
	tv.tv_sec = 0x178;
	tv.tv_usec = 0x357;
	uint64_t ntp = hbk::sys::TimeConvert::unixtime2Ntp(tv);
	
	uint32_t usec = hbk::sys::TimeConvert::ntpfraction2microseconds(ntp);
	uint64_t sec = ntp;
	sec >>= 32;
	
	ASSERT_EQ(sec-hbk::sys::TimeConvert::ntpSecondOffset, tv.tv_sec);
	ASSERT_EQ(usec, tv.tv_usec);
}

TEST(test_timeutil,  unixtime2NtpStruct_test)
{
	static const double unixTime = 1.5;
	hbk::sys::TimeConvert::NtpStruct ntpStruct;
	ntpStruct = hbk::sys::TimeConvert::unixtime2NtpStruct(unixTime);

	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 500000;
	ntpStruct = hbk::sys::TimeConvert::unixtime2NtpStruct(tv);
	ASSERT_EQ(ntpStruct.seconds-hbk::sys::TimeConvert::ntpSecondOffset,1);
	ASSERT_EQ(ntpStruct.fractions, 0x80000000);
}
