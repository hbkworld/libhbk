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

#include "hbk/exception/jsonrpc_exception.h"
#include "hbk/jsonrpc/jsonrpc_defines.h"

namespace hbk {
	namespace exception {
		jsonrpcException::jsonrpcException( int excCode, const std::string& excMessage)
			: exception("")
			, m_code(excCode)
			, m_message(excMessage)
		{
			m_what = "code: " + std::to_string(m_code);
			if (!m_message.empty()) {
				m_what += ", message: " + m_message;
			}
		}

		int jsonrpcException::code() const noexcept
		{
			return m_code;
		}

		std::string jsonrpcException::message() const noexcept
		{
			return m_message;
		}

		const char* jsonrpcException::what() const noexcept
		{
			return m_what.c_str();
		}
	}
}
