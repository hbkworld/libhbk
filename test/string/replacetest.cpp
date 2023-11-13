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
#include <vector>

#include <gtest/gtest.h>

#include <iostream>

#include "hbk/string/replace.h"


namespace hbk {
	namespace string {
		namespace test {
			TEST(replacetest, test_case_replace_char)
			{
				std::string result = hbk::string::replace("abc:defgh:ijkl", ':', ' ');
				ASSERT_EQ(result, "abc defgh ijkl");
				result = hbk::string::replace("1.2.3", '.', ' ');
				ASSERT_EQ(result, "1 2 3");

				// replace with same should not end in infinite loop!
				result = hbk::string::replace("1.2.3", '.', '.');
			}

			TEST(replacetest, test_case_replace_string)
			{
				std::string result = hbk::string::replace("abc::defgh::ijkl", "::", "  ");
				ASSERT_EQ(result, "abc  defgh  ijkl");
				result = hbk::string::replace("1..2..3", "..", "  ");
				ASSERT_EQ(result, "1  2  3");

				// replace with same should not end in infinite loop!
				result = hbk::string::replace("1..2..3", "..", "..");
				ASSERT_EQ(result, "1..2..3");
			}
		}
	}
}
