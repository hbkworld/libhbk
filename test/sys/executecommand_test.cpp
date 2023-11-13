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
#include <iostream>

#include <gtest/gtest.h>

#include "hbk/sys/executecommand.h"
#include "hbk/exception/exception.hpp"


TEST(executecommand, command_withparameters_test)
{
	static const std::string fileName = "bla";
	::remove(fileName.c_str());
	hbk::sys::executeCommand("/usr/bin/touch " + fileName);
	int result = ::remove(fileName.c_str());
	ASSERT_TRUE(result==0);
}


TEST(executecommand, invalid_command_test)
{
	static const std::string fileName = "bla";
	hbk::sys::params_t params;
	params.push_back(fileName);
	int result = hbk::sys::executeCommand("/usr/bin/touc", params, "");
	ASSERT_TRUE(result==-1);
}

TEST(executecommand, valid_command_test)
{
	static const std::string fileName = "bla";
	::remove(fileName.c_str());
	hbk::sys::params_t params;
	params.push_back(fileName);

	int result = hbk::sys::executeCommand("/usr/bin/touch", params, "");
	ASSERT_TRUE(result==0);
	result = ::remove(fileName.c_str());
	ASSERT_TRUE(result==0);
}

TEST(executecommand, stdin_test)
{
	hbk::sys::params_t params;

	params.push_back("-w");
	int result = hbk::sys::executeCommand("/usr/bin/wc", params, "bla blub");
	ASSERT_TRUE(result==0);
}

TEST(executecommand, answer_test)
{
	std::string cmd = "/bin/echo";
	std::string arg = "hallo";
	std::string result = hbk::sys::executeCommand(cmd + " " + arg);
	ASSERT_TRUE(result.substr(0, result.size()-1)==arg);
}
