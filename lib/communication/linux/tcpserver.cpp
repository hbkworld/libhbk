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

#include <memory>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <errno.h>
#include <syslog.h>

#include "hbk/communication/socketnonblocking.h"
#include "hbk/communication/tcpserver.h"
#include "hbk/sys/eventloop.h"


namespace hbk {
	namespace communication {
		TcpServer::TcpServer(sys::EventLoop &eventLoop)
			: m_listeningEvent(-1)
			, m_eventLoop(eventLoop)
			, m_acceptCb()
		{
		}

		TcpServer::~TcpServer()
		{
			stop();
		}

		int TcpServer::start(uint16_t port, int backlog, Cb_t acceptCb)
		{
			if (!acceptCb) {
				return -1;
			}

			//ipv6 does work for ipv4 too!
			sockaddr_in6 address;
			memset(&address, 0, sizeof(address));
			address.sin6_family = AF_INET6;
			address.sin6_addr = in6addr_any;
			address.sin6_port = htons(port);

			m_listeningEvent = ::socket(address.sin6_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
			if (m_listeningEvent==-1) {
				::syslog(LOG_ERR, "server: Socket initialization failed '%s'", strerror(errno));
				return -1;
			}
			
			uint32_t yes = 1;
			// important for start after stop. Otherwise we have to wait some time until the port is really freed by the operating system.
			if (setsockopt(m_listeningEvent, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
				::syslog(LOG_ERR, "server: Could not set SO_REUSEADDR!");
				return -1;
			}
			
			if (::bind(m_listeningEvent, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
				::syslog(LOG_ERR, "server: Binding socket to port %u failed '%s'", port, strerror(errno));
				return -1;
			}
			if (listen(m_listeningEvent, backlog)==-1) {
				return -1;
			}
			m_acceptCb = acceptCb;
			m_eventLoop.addEvent(m_listeningEvent, std::bind(&TcpServer::process, this));
			return 0;
		}

		int TcpServer::start(const std::string& path, bool useAbstractNamespace, int backlog, Cb_t acceptCb)
		{
			sockaddr_un address;
			memset(&address, 0, sizeof(address));
			address.sun_family = AF_UNIX;

			size_t serveraddrLen;
			if(useAbstractNamespace) {
				serveraddrLen = 1+strlen(path.c_str())+sizeof(address.sun_family);
				strncpy(&address.sun_path[1], path.c_str(), sizeof(address.sun_path)-2);
			} else {
				serveraddrLen = strlen(path.c_str())+sizeof(address.sun_family);
				strncpy(address.sun_path, path.c_str(), sizeof(address.sun_path)-1);
				::unlink(path.c_str());
			}

			m_listeningEvent = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
			if (m_listeningEvent==-1) {
				::syslog(LOG_ERR, "server: Socket initialization failed '%s'", strerror(errno));
				return -1;
			}

			uint32_t yes = 1;
			// important for start after stop. Otherwise we have to wait some time until the port is really freed by the operating system.
			if (setsockopt(m_listeningEvent, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
				::syslog(LOG_ERR, "server: Could not set SO_REUSEADDR!");
				return -1;
			}

			if (::bind(m_listeningEvent, reinterpret_cast<sockaddr*>(&address), serveraddrLen) == -1) {
				::syslog(LOG_ERR, "server: Binding socket to unix domain socket %s failed '%s'", path.c_str(), strerror(errno));
				return -1;
			}

			if(!useAbstractNamespace) {
				chmod(path.c_str(), 0666); // everyone should have access
				m_unixDomainSocketPath = path;
			}
			if (listen(m_listeningEvent, backlog)==-1) {
				return -1;
			}

			m_acceptCb = acceptCb;
			m_eventLoop.addEvent(m_listeningEvent, std::bind(&TcpServer::process, this));
			return 0;
		}

		void TcpServer::stop()
		{
			m_eventLoop.eraseEvent(m_listeningEvent);
			::close(m_listeningEvent);
			if (!m_unixDomainSocketPath.empty()) {
				// unlink non-abstract unix domain socket
				if(::unlink(m_unixDomainSocketPath.c_str())) {
					::syslog(LOG_ERR, "server: unlinking %s failed '%s'", m_unixDomainSocketPath.c_str(), strerror(errno));
				}
			}
			m_acceptCb = Cb_t();
		}

		int TcpServer::process()
		{
			int clientFd = ::accept(m_listeningEvent, nullptr, nullptr);
			if (clientFd==-1) {
				if ((errno!=EWOULDBLOCK) && (errno!=EAGAIN) && (errno!=EINTR) ) {
					::syslog(LOG_ERR, "server: error accepting connection '%s'", strerror(errno));
				}
				return -1;
			}
			m_acceptCb(clientSocket_t(new SocketNonblocking(clientFd, m_eventLoop)));
			// we are working edge triggered. Returning > 0 tells the eventloop to call process again to try whether there is more in the queue.
			return 1;
		}
	}
}
