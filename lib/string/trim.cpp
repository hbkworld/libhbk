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

#include <string>

#include "hbk/string/trim.h"


namespace hbk {
	namespace string {
		std::string trim_copy(std::string text)
		{
			std::string::size_type start = text.find_first_not_of(trim_ws);
			if (start == std::string::npos) {
				return "";
			}
			std::string::size_type end = text.find_last_not_of(trim_ws);
			std::string::size_type length = end-start+1;

			return text.substr(start, length);
		}

		void trim_left(std::string& text)
		{
			std::string::size_type start = text.find_first_not_of(trim_ws);
			text.erase(0, start);
		}

		void trim_right(std::string& text)
		{
			std::string::size_type end = text.find_last_not_of(trim_ws);
			if (end == std::string::npos) {
				// special case: the only character does match!
				if (text[0]==' ') {
					text.clear();
				}
				return;
			}
			text.erase(end+1);
		}

		void trim(std::string& text)
		{
			trim_left(text);
			trim_right(text);
		}

		void trim_left_if(std::string& text, char ch)
		{
			std::string::size_type start = text.find_first_not_of(ch);
			text.erase(0, start);
		}

		void trim_right_if(std::string& text, char ch)
		{
			std::string::size_type end = text.find_last_not_of(ch);
			if (end == std::string::npos) {
				// special case: the only character does match!
				if (text[0]==ch) {
					text.clear();
				}
				return;
			}
			text.erase(end+1);
		}

	}
}

