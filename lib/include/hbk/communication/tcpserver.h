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

#ifndef __HBK__COMMUNICATION_TCPACCEPTOR_H
#define __HBK__COMMUNICATION_TCPACCEPTOR_H

#include <memory>
#include <string>

#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2ipdef.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#endif



#include "hbk/communication/socketnonblocking.h"
#include "hbk/sys/eventloop.h"

namespace hbk {
	namespace communication {
		/// Listens for and accepts incoming connections from clients. A callback function will be called with the worker socket for the accepted client.
		/// Under Linux ipv4 and ipv6 are supported. Under Windows ipv4 is supported.
		class TcpServer {
		public:
			/// deliveres the worker socket for an accepted client
			using Cb_t = std::function < void (clientSocket_t) >;

			/// @param eventLoop Event loop the object will be registered in 
			TcpServer(sys::EventLoop &eventLoop);
			virtual ~TcpServer();

			/// @param port TCP port to listen to
			/// @param backlog Maximum length of the queue of pending connections
			/// @param acceptCb called when accepting a new tcp client
			/// \return -1 on error
			int start(uint16_t port, int backlog, Cb_t acceptCb);

			/// @param path path of unix domain socket to listen to
			/// @param backlog Maximum length of the queue of pending connections
			/// @param acceptCb called when accepting a new tcp client
			/// \return -1 on error
			int start(const std::string& path, int backlog, Cb_t acceptCb);

			/// Remove this object from the event loop and close the server socket
			void stop();

		private:

			/// should not be copied
			TcpServer(const TcpServer& op);

			/// should not be assigned
			TcpServer& operator= (const TcpServer& op);

			/// called by eventloop
			/// accepts a new connection creates new worker socket anf calls acceptCb
			int process();

#ifdef _WIN32
			/// accepts a new connecting client.
			/// \return On success, the worker socket for the new connected client is returned. Empty worker socket on error
			clientSocket_t acceptClient();

			int prepareAccept();
#endif

			sys::event m_listeningEvent;
#ifdef _WIN32
			SOCKET m_acceptSocket;

			LPFN_ACCEPTEX m_acceptEx;
			/// be prepared for ipv6
			char m_acceptBuffer[2*(sizeof(sockaddr_in6)+16)];
			DWORD m_acceptSize;
#endif
			sys::EventLoop& m_eventLoop;
			Cb_t m_acceptCb;

			std::string m_path;
		};
	}
}
#endif
