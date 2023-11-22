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

#ifndef __HBK__STRING__TRIM_H
#define __HBK__STRING__TRIM_H

#include <string>

namespace hbk {
	namespace string {
		/// Default whitespace characters
		static const char trim_ws[] = " \t\n\r\f\v";

		/// right and left trim of default whitespace characters
		std::string trim_copy(std::string text);

		/// left trim of default whitespace characters
		void trim_left(std::string& text);

		/// right trim of default whitespace characters
		void trim_right(std::string& text);

		/// right and left trim of default whitespace characters
		void trim(std::string& text);

		/// left trim of character ch
		void trim_left_if(std::string& text, char ch);

		/// right trim of character ch
		void trim_right_if(std::string& text, char ch);
	}
}
#endif // __HBK__STRING__TRIM_H
