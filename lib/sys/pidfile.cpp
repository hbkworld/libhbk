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

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <errno.h>

#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#define basename(x) x
#else
#include <unistd.h>
#include <libgen.h>
#endif


#include "hbk/sys/pidfile.h"

#ifdef _STANDARD_HARDWARE
	// use current directory on pc because we hopefully have write access here.
	const char BasePath[] = "";
#else
	const char BasePath[] = "/var/run/";
#endif

namespace hbk {
	namespace sys {
		PidFile::PidFile(const char* name)
			: m_pidFileName(BasePath)
		{
#ifdef _WIN32
			char* pNameCopy = _strdup(name);
#else
			char* pNameCopy = strdup(name);
#endif
			m_pidFileName += basename(pNameCopy);
			free(pNameCopy);
			m_pidFileName += ".pid";
			FILE* pidFile = ::fopen(m_pidFileName.c_str(), "w");

			if (pidFile == nullptr) {
				std::string msg;
				msg = "could not create pid file ";
				msg += m_pidFileName;
				msg += std::string(" '") +strerror(errno) + "'";
				throw std::runtime_error(msg);
			} else {
				::fprintf(pidFile, "%d\n", getpid());
				::fclose(pidFile);
			}
		}

		std::string PidFile::path()
		{
			return m_pidFileName;
		}

		PidFile::~PidFile()
		{
			::remove(m_pidFileName.c_str());
		}
	}
}
