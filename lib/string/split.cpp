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

#include "hbk/string/split.h"


namespace hbk {
	namespace string {

		tokens split(const std::string& text, char separator)
		{
			tokens result;

			size_t pos_start = 0;

			while(1) {
				size_t pos_end = text.find(separator, pos_start);
				std::string token = text.substr(pos_start, pos_end-pos_start);
				result.push_back(token);
				if (pos_end == std::string::npos) {
					break;
				}
				pos_start = pos_end+1;
			}
			return result;
		}

		tokens split(const std::string& text, const std::string& separator)
		{
			tokens result;
			size_t separatorLength = separator.length();

			if(separatorLength == 0) {
				result.push_back(text);
				return result;
			}

			size_t pos_start = 0;

			while(1) {
				size_t pos_end = text.find(separator, pos_start);
				std::string token = text.substr(pos_start, pos_end - pos_start);
				result.push_back(token);
				if (pos_end == std::string::npos) {
					break;
				}
				pos_start = pos_end + separatorLength;
			}
			return result;
		}
	}
}

