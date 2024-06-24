// Copyright 2022 Hottinger Brüel & Kjær
// Distributed under MIT license
// See file LICENSE provided

#include <fstream>


#include <gtest/gtest.h>


#include "hbk/string/readlinefromfile.h"


namespace hbk {
	namespace string {
		namespace test {

			TEST(stringtest, readlinefromfile_test)
			{
					std::string path = "theFile";
					std::string content;
					std::string result;
					
					{
							// non-existent file
							EXPECT_THROW(readLineFromFile(""), std::runtime_error);
					}
					
					{
							// file having a valid string
							remove(path.c_str());
							content = "a string";
							
							std::ofstream file;
							file.open(path);
							file << content;
							file.close();
							result = readLineFromFile(path);
							ASSERT_EQ(result, content);
					}
					
					{
							// file having a '\0' in it
							remove(path.c_str());
							content = "a string";
							std::vector < uint8_t > binaryContent(content.length()+1);
							memcpy(binaryContent.data(), content.c_str(), content.length());
							binaryContent[content.length()] = '\0';
							
							// insert '\0'
							binaryContent[content.length()/2] = '\0';
							
							std::ofstream file;
							file.open(path);
							file.write(reinterpret_cast< char* >(binaryContent.data()), binaryContent.size());
							file.close();
							result = readLineFromFile(path);
							std::string contentPart = content.substr(0, content.size()/2);
							ASSERT_EQ(result, contentPart);
					}
			}
		}
	}
}
