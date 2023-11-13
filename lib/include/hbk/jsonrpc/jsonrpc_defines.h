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

#ifndef _HBK__JSON__JSONNAME_H
#define _HBK__JSON__JSONNAME_H
namespace hbk {
	namespace jsonrpc {
		/// some error codes from the JSON RPC spec
		static const int parseError = -32700;
		static const int invalidRequest = -32600;
		static const int methodNotFound = -32601;
		static const int invalidParams = -32602;
		static const int internalError = -32603;

		/// some string constants from the JSON RPC spec
		static const char JSONRPC[] = "jsonrpc";
		static const char METHOD[] = "method";
		static const char RESULT[] = "result";
		static const char ERR[] = "error";
		static const char CODE[] = "code";
		static const char MESSAGE[] = "message";
		static const char DATA[] = "data";
		static const char PARAMS[] = "params";
		static const char ID[] = "id";

		// example of an successfull rpc
		// --> {"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 1}
		// <-- {"jsonrpc": "2.0", "result": 19, "id": 1}

		// example of a failes
		//--> {"jsonrpc": "2.0", "method": "foobar", "id": "1"}
		//<-- {"jsonrpc": "2.0", "error": {"code": -32601, "message": "Method not found"}, "id": "1"}
	}
}
#endif
