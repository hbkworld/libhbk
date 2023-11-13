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

#ifndef __HBK__STRING__SPLIT_H
#define __HBK__STRING__SPLIT_H

#include <string>
#include <vector>

namespace hbk {
	namespace string {
		using tokens = std::vector < std::string >;

		/// \brief split a string by a separator-string
		///
		/// The string text is searched for occurenced of the substring separator
		/// from left ro right.
		/// <br>
		/// If text is an empty string, a vector with one member is returned,
		/// which is the empty string.
		/// <br>
		/// If text does not contain the separator, a vector with one member is
		/// returned, which is the original string text.
		///
		/// \param text  the string to split
		/// \param separator  the string to search for in the text
		/// \return vector of all pieces of text resulted by chopping it at each separator.
		tokens split(const std::string& text, const std::string& separator);

		tokens split(const std::string& text, char separator);
	}
}

#endif // __HBK__STRING__SPLIT_H
