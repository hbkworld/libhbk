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

#ifndef __HBK_SYS_TIMECONVERT_H_
#define __HBK_SYS_TIMECONVERT_H_

#include <stdint.h>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif

namespace hbk {
	namespace sys {
		class TimeConvert {
		public:
			/// according to RFC 868:
			/// offset between the ntp epoch 1.1.1900 and the unix epoch 1.1.1970
			static const uint32_t ntpSecondOffset;

			struct NtpStruct {
				uint32_t seconds;
				uint32_t fractions;
			};

			/// \return lower 32 bit of ntp time stamp
			static uint32_t microseconds2Ntpfraction(uint32_t microseconds);

			/// \return lower 32 bit of ntp time stamp
			static uint32_t nanoseconds2Ntpfraction(uint32_t nanoseconds);
			static NtpStruct nanoseconds2NtpStruct(uint64_t nanoseconds);

			/// seconds of ntp time are ntpSecondOffset bigger than sceonds of unix time
			static uint64_t unixtime2Ntp(double unixTime);

			/// seconds of ntp time are ntpSecondOffset bigger than sceonds of unix time
			static NtpStruct unixtime2NtpStruct(double unixTime);

			/// seconds of ntp time are ntpSecondOffset bigger than sceonds of unix time
			static uint64_t unixtime2Ntp(struct timeval unixTime);

			/// seconds of ntp time are ntpSecondOffset bigger than sceonds of unix time
			static TimeConvert::NtpStruct unixtime2NtpStruct(timeval unixTime);

			/// \param ntpfraction lower 32 bit of ntp time stamp
			static uint32_t ntpfraction2microseconds(uint32_t ntpfraction);

			/// \param ntpfraction lower 32 bit of ntp time stamp
			static uint32_t ntpfraction2nanoseconds(uint32_t ntpfraction);

			/// seconds of unix time are ntpSecondOffset smaller than sceonds of ntp time
			static double ntp2double(uint64_t ntp);
		};

	}
}

#endif
