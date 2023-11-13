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

#ifndef HBK_EXCEPTION_H
#define HBK_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace hbk {
	namespace exception {
		/// base class for all exceptions raised/thrown by code in this repo
		/// this base class is required for catching all hbk-specific exceptions with
		/// a catch clause like
		/// try
		/// {
		/// }
		/// catch( const hbk::exception& e )
		/// {
		/// }
		class exception : public std::runtime_error {
			public:
				exception(const std::string& description);
				virtual ~exception() noexcept;
				virtual const char* what() const noexcept;
			protected:
				std::string output;

			private:
				static const unsigned int backtrace_size = 100;
		};
	}
}

#endif

