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


#include "hbk/string/trim.h"


namespace hbk {
	namespace string {
		namespace test {



			TEST(trimtest_copy,  test_case_right )
			{
				std::string result = trim_copy("hallo  \t");
				ASSERT_EQ(result, "hallo");
			}

			TEST(trimtest_copy,  test_case_left )
			{
				std::string result = trim_copy("\f  hallo");
				ASSERT_EQ(result, "hallo");
			}

			TEST(trimtest_copy,  test_case_both )
			{
				std::string result = trim_copy(" \v hallo \n ");
				ASSERT_EQ(result, "hallo");
			}

			TEST(trimtest_copy,  test_case_nothing )
			{
				std::string result = trim_copy("hallo");
				ASSERT_EQ(result, "hallo");
			}

			TEST(trimtest_copy,  test_case_empty )
			{
				std::string result = trim_copy("  \r  ");
				ASSERT_EQ(result, "");
			}




			TEST(trimtest,  test_case_right )
			{
				std::string text = "hallo  ";
				trim(text);
				ASSERT_EQ(text, "hallo");

				text = " ";
				trim_right(text);
				ASSERT_EQ(text, "");
			}
			
			TEST(trimtest,  test_case_xxx_if )
			{
				std::string text = " ";
				trim_right_if(text, ' ');
				ASSERT_EQ(text, "");

				text = " h ";
				trim_right_if(text, ' ');
				ASSERT_EQ(text, " h");

				text = " h ";
				trim_right_if(text, 'a');
				ASSERT_EQ(text, " h ");

				text = " h ";
				trim_left_if(text, ' ');
				ASSERT_EQ(text, "h ");

				text = " h ";
				trim_left_if(text, 'a');
				ASSERT_EQ(text, " h ");
			}

			TEST(trimtest,  test_case_left )
			{
				std::string text = "  hallo";
				trim(text);
				ASSERT_EQ(text, "hallo");
			}

			TEST(trimtest,  test_case_both )
			{
				std::string text = "  hallo  ";
				trim(text);
				ASSERT_EQ(text, "hallo");
			}

			TEST(trimtest,  test_case_nothing )
			{
				std::string text = "hallo";
				trim(text);
				ASSERT_EQ(text, "hallo");
			}

			TEST(trimtest,  test_case_empty )
			{
				std::string text = "    ";
				trim(text);
				ASSERT_EQ(text, "");
			}
		}
	}
}
