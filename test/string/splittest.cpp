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

#include "hbk/string/split.h"


namespace hbk {
	namespace string {
		namespace test {
			TEST(replacetest, test_case_common)
			{
				ASSERT_EQ(split("abc:defgh:ijkl", ":"), tokens({"abc","defgh","ijkl"}));
				ASSERT_EQ(split("1.2.3", "."), tokens({"1","2","3"}));
			}
			
			TEST(replacetest, test_case_empty)
			{
				ASSERT_EQ(split("", ":"), tokens({""}));
				ASSERT_EQ(split("abd:defgh:ijkl", ""), tokens({"abd:defgh:ijkl"}));
			}
			
			TEST(replacetest, test_char_pattern)
			{
				ASSERT_EQ(split("abc:de:fgh:", ':'), tokens({"abc", "de", "fgh", ""}));
			}
			
			TEST(replacetest, test_case_pattern)
			{
				ASSERT_EQ(split("abd:de::fgh:ijk::l", "::"), tokens({"abd:de","fgh:ijk","l"}));
				
				ASSERT_EQ(split("::a::bc::def::g::", "::"), tokens({"","a","bc","def","g",""}));
				ASSERT_EQ(split("........", ".."), tokens({"","","","",""}));
				ASSERT_EQ(split("........", "..."), tokens({"","",".."}));
				ASSERT_EQ(split("........", "...."), tokens({"","",""}));
				ASSERT_EQ(split("........", "....."), tokens({"","..."}));
			}
			
			TEST(replacetest, test_case_split_at_ends)
			{
				ASSERT_EQ(split(".1234.5678", "."), tokens({"","1234","5678"}));
				ASSERT_EQ(split(".12345678", "."), tokens({"","12345678"}));
				ASSERT_EQ(split("1234.5678.", "."), tokens({"1234","5678",""}));
				ASSERT_EQ(split("12345678.", "."), tokens({"12345678",""}));
				ASSERT_EQ(split(".1234.5678.", "."), tokens({"","1234","5678",""}));
				ASSERT_EQ(split("1234.5678", "."), tokens({"1234","5678"}));
				ASSERT_EQ(split("12345678", "."), tokens({"12345678"}));
			}
			
			TEST(replacetest, test_case_split_at_ends2)
			{
				ASSERT_EQ(split("...","."), tokens({"","","",""}));
				ASSERT_EQ(split(".", "."), tokens({"",""}));
			}
		}
	}
}
