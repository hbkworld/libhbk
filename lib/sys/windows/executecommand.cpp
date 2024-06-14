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

#include <iostream>

#include "hbk/sys/executecommand.h"

namespace hbk {
	namespace sys {
		std::string executeCommand(const std::string& command)
		{
			std::string retVal;
#ifdef _STANDARD_HARDWARE
			std::cout << command << std::endl;
#else
			// todo: for windows
#endif
			return retVal;
		}

		int executeCommand(const std::string& command, const params_t &params, const std::string& stdinString)
		{
#ifdef _STANDARD_HARDWARE
			std::cout << command << " ";

			for(params_t::const_iterator iter = params.cbegin(); iter!=params.cend(); ++iter) {
				std::cout << *iter << " ";
			}

			std::cout << " < " << stdinString << std::endl;
#else
			// todo: for windows
#endif
			return 0;
		}
	}
}

