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

#include <cmath>
#include <stdint.h>

#include "hbk/sys/timeconvert.h"

namespace hbk {
	namespace sys {
	const uint32_t TimeConvert::ntpSecondOffset = 2208988800;

	uint32_t TimeConvert::microseconds2Ntpfraction(uint32_t microseconds)
	{
		// ntpfraction = microseconds * 2^32 / 1000000
		uint64_t res =microseconds;
		res <<= 32;
		res /= 1000000;
		return static_cast < uint32_t > (res);
	}

	uint32_t TimeConvert::nanoseconds2Ntpfraction(uint32_t nanoseconds)
	{
		// ntpfraction = nanoseconds * 2^32 / 1000000
		uint64_t res =nanoseconds;
		res <<= 32;
		res /= 1000000000;
		return static_cast < uint32_t > (res);
	}

	TimeConvert::NtpStruct TimeConvert::nanoseconds2NtpStruct(uint64_t nanoseconds)
	{
		TimeConvert::NtpStruct ntp;
		// ntpfraction = nanoseconds * 2^32 / 1000000
		uint64_t res =nanoseconds;
		res <<= 32;
		ntp.seconds = static_cast < uint32_t > (nanoseconds / 1000000000);
		ntp.fractions = static_cast < uint32_t > (res / 1000000000);
		return ntp;
	}

	uint64_t TimeConvert::unixtime2Ntp(double unixTime) {
		uint64_t ntptimestamp;
		uint32_t fraction;
		uint32_t seconds;
		double secondsDouble;
		double fractionDoulbe = modf(unixTime, &secondsDouble);
		seconds = static_cast < uint32_t >(secondsDouble);
		fraction = static_cast < uint32_t >(fractionDoulbe * 0x100000000);
		ntptimestamp = seconds+ntpSecondOffset;
		ntptimestamp <<= 32;
		ntptimestamp |= fraction;
		return ntptimestamp;
	}

	TimeConvert::NtpStruct TimeConvert::unixtime2NtpStruct(double unixTime)
	{
		NtpStruct ntpStruct;

		double secondsDouble;
		double fractionDoulbe = modf(unixTime, &secondsDouble);
		ntpStruct.seconds = static_cast < uint32_t >(secondsDouble)+ntpSecondOffset;
		ntpStruct.fractions = static_cast < uint32_t >(fractionDoulbe * 0x100000000);
		return ntpStruct;
	}

	uint64_t TimeConvert::unixtime2Ntp(timeval unixTime)
	{
		uint64_t ntptimestamp;
		ntptimestamp = unixTime.tv_sec+ntpSecondOffset;
		ntptimestamp <<= 32;
		ntptimestamp |= microseconds2Ntpfraction(unixTime.tv_usec);
		return ntptimestamp;
	}

	TimeConvert::NtpStruct TimeConvert::unixtime2NtpStruct(timeval unixTime)
	{
		NtpStruct ntp;
		ntp.seconds = unixTime.tv_sec+ntpSecondOffset;
		ntp.fractions = microseconds2Ntpfraction(unixTime.tv_usec);
		return ntp;
	}

	uint32_t TimeConvert::ntpfraction2microseconds(const uint32_t ntpfraction)
	{
		// microseconds = (ntpfraction+1) * 10^6 / 2^32
		uint64_t res = ntpfraction + 1;
		res *= 1000000;
		res >>= 32;
		return static_cast < uint32_t > (res);
	}

	uint32_t TimeConvert::ntpfraction2nanoseconds(const uint32_t ntpfraction)
	{
		// nanoseconds = (ntpfraction+1) * 10^9 / 2^32
		uint64_t res = ntpfraction;
		res++;
		res *= 1000000000;
		res >>= 32;
		return static_cast < uint32_t > (res);
	}

	double TimeConvert::ntp2double(uint64_t ntp) {
		double valueDouble;
		double fractionDoulbe = static_cast < uint32_t > (ntp) & 0xffffffff;
		fractionDoulbe /= 0x100000000;
		valueDouble = static_cast < uint32_t > (ntp >> 32);
		valueDouble -= ntpSecondOffset;
		valueDouble += fractionDoulbe;
		return valueDouble;
	}
	}
}
