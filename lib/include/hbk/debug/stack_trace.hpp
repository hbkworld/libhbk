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


#ifndef stack_trace_hpp_
#define stack_trace_hpp_


#include <string>

#ifdef __GNUG__
#include <execinfo.h>
#endif

namespace hbk {
	namespace debug {

		static const unsigned int backtrace_size = 100;

		static std::string fill_stack_trace()
		{
			std::string output;
#ifdef __GNUG__
			void *backtrace_buffer[backtrace_size];
			int num_functions = ::backtrace(backtrace_buffer, backtrace_size);
			char **function_strings = ::backtrace_symbols(backtrace_buffer, num_functions);
			if (function_strings != nullptr) {
				output.append("\n----------- stacktrace begin\n");
				output.append("\n");
				for (int i = 0; i < num_functions; ++i) {
					output.append(function_strings[i]);
					output.append("\n");
				}
				output.append("----------- stacktrace end\n");
				::free(function_strings);
			} else {
				output.append("No backtrace!\n");
			}
#else
			output.append("No backtrace!\n");
#endif
			return output;
		}
	}
}


#endif
