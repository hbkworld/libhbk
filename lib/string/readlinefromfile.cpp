// Copyright 2022 Hottinger Brüel & Kjær
// Distributed under MIT license
// See file LICENSE provided

#include <algorithm>
#include <fstream>
#include <string>

#include "hbk/string/readlinefromfile.h"

namespace hbk {
	namespace string {
		std::string readLineFromFile(const std::string& path)
		{
			std::ifstream file;
			file.open(path);
			if (!file) {
				throw std::runtime_error(std::string("Could not open file %s for reading") + path.c_str());
			}
			std::string line;
			std::getline(file, line);
			line.erase(std::find(line.begin(), line.end(), '\0'), line.end());
			file.close();
			return line;
		}
	}
}
