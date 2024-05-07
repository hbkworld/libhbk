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
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <array>
#include <vector>

#include <sys/types.h>


#ifdef _WIN32
#define syslog fprintf
#define LOG_ERR stderr
#define popen(a, b) _popen(a, b)
#define pclose(a) _pclose(a)
#else
#include <wait.h>
#include <syslog.h>
#include <unistd.h>
#endif

#include <errno.h>

#include "hbk/exception/exception.hpp"
#include "hbk/sys/executecommand.h"

namespace hbk {
	namespace sys {
		/// Callback for automatic destruction of unique pointer
		static void closePipe(std::FILE* fp)
		{
			pclose(fp);
		};

		std::string executeCommand(const std::string& command)
		{
			std::string retVal;
			std::unique_ptr<FILE, decltype(&closePipe)> pipe(popen(command.c_str(), "r"), closePipe);
			if (!pipe) {
				std::string msg = std::string(__FUNCTION__) + "popen failed (cmd=" + command + ")!";
				throw hbk::exception::exception(msg);
			} else {
				std::array < char, 1024> buffer;
				char* pBuffer = buffer.data();
				do {
					size_t count = fread(pBuffer, 1, sizeof(buffer), pipe.get());
					if (count == 0) {
						break;
					}
					retVal += std::string(pBuffer, count);
				} while(true);
			}
			return retVal;
		}

		int executeCommand(const std::string& command, const params_t &params, const std::string& stdinString)
		{
			static const unsigned int PIPE_READ = 0;
			static const unsigned int PIPE_WRITE = 1;
			int pfd[2];
			pid_t cpid;

			if (pipe(pfd) == -1){
				syslog(LOG_ERR, "error creating pipe");
				return -1;
			}
			cpid = fork();
			if (cpid == -1) {
				syslog(LOG_ERR, "error forking process");
				return -1;
			}
			if (cpid == 0) {
				// Child
				close(pfd[PIPE_WRITE]); // close unused write end

				// redirect stdin
				if(pfd[PIPE_READ] != STDIN_FILENO) {
					if (dup2(pfd[PIPE_READ], STDIN_FILENO) == -1) {
						syslog(LOG_ERR, "error redirecting stdin");
						return -1;
					}
				}

				std::vector < char* > argv;
				argv.push_back(const_cast < char* > (command.c_str()));
				for (const std::string& iter: params) {
					argv.push_back( const_cast < char* > (iter.c_str()));
				}
				argv.push_back(nullptr);

				execve(command.c_str(), &argv[0], nullptr);
				// if we get here at all, an error occurred, but we are in the child
				// process, so just exit
				syslog(LOG_ERR, "error executing '%s' '%s'", command.c_str(), strerror(errno));
				exit(EXIT_FAILURE);
			} else if ( cpid > 0 ) {
				int waitStatus;
				// Parent
				close(pfd[PIPE_READ]); // close unused read end

				// send data to stdin of child
				const char* pStdinData = stdinString.c_str();
				size_t stdinDataSize = stdinString.size();
				while(stdinDataSize) {
					ssize_t ret = write(pfd[PIPE_WRITE], pStdinData, stdinDataSize);
					if (ret == -1) {
						syslog(LOG_ERR, "error writing to stdin of child '%s'", command.c_str());
						break;
					}
					stdinDataSize -= static_cast < size_t > (ret);
					pStdinData += ret;
				}
				close(pfd[PIPE_WRITE]);

				// wait for child to finish
				waitpid(cpid, &waitStatus, 0);

				if(WIFEXITED(waitStatus)==false) {
					// child process did not exit normally
					return -1;
				}

				if(WEXITSTATUS(waitStatus)!=EXIT_SUCCESS) {
					// child did exit with failure. Maybe the desired program could not be executed. Otherwise the executed program itself returned the error.
					return -1;
				}
			} else {
				syslog(LOG_ERR, "failed to create child!");
				close(pfd[PIPE_READ]);
				close(pfd[PIPE_WRITE]);
				return -1;
			}
			return 0;
		}
	}
}

