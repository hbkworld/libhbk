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

#ifndef __HBK__SOCKETNONBLOCKING_H
#define __HBK__SOCKETNONBLOCKING_H

#include <list>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#ifndef ssize_t
typedef int ssize_t;
#endif
#else
#include <sys/socket.h>
#endif

#include "hbk/communication/bufferedreader.h"
#include "hbk/sys/eventloop.h"

namespace hbk
{
	namespace communication {
		/// used for scatter gather operations
		struct dataBlock_t {
			dataBlock_t()
				: pData(nullptr)
				, size(0)
			{
			}

			/// All members are initialized on construction
			dataBlock_t(const void* pD, size_t s)
				: pData(pD)
				, size(s)
			{
			}

			/// Data buffer
			const void* pData;
			/// Size of the data buffer
			size_t size;
		};

		using dataBlocks_t = std::list < dataBlock_t >;

		/// A tcp client connection to a tcpserver. Ipv4 and ipv6 are supported.
		/// the socket uses keep-alive in order to detect broken connection.
		class SocketNonblocking
		{
		public:
			/// called on the arrival of data
			using DataCb_t = std::function < ssize_t (SocketNonblocking& socket) >;
			/// @param eventLoop Event loop the object will be registered in. A running eventloop is necessary to handle input/output events.
			/// A running eventloop is not necessary if you are just using methods for receiving or sending data.
			SocketNonblocking(sys::EventLoop &eventLoop);


			/// not movable
			SocketNonblocking(SocketNonblocking&& op) = delete;
			/// not movable
			SocketNonblocking& operator= (SocketNonblocking&& op) = delete;

			/// should not be copied
			SocketNonblocking(const SocketNonblocking& op) = delete;
			/// should not be assigned
			SocketNonblocking& operator= (const SocketNonblocking& op) = delete;


			/// used when accepting connection via tcp server.
			/// \throw std::runtime_error on error
			SocketNonblocking(int fd, sys::EventLoop &eventLoop);
			virtual ~SocketNonblocking();

			/// this method does work blocking
			/// \param address address of tcp server or path od unix domain socket
			/// \param port tcp port to connect to
			/// \return 0: success; -1: error
			int connect(const std::string& address, const std::string& port);

#ifndef _WIN32
			/// this method does work blocking
			/// \param path path to unix domain socket
			/// \param useAbstractNamespace true unix domain socket is in abstract namespace
			/// \return 0: success; -1: error
			int connect(const std::string &path, bool useAbstractNamespace = true);
#endif

			/// this method does work blocking
			int connect(int domain, const struct sockaddr* pSockAddr, socklen_t len);

			/// Remove event from event loop and close socket
			void disconnect();

			/// if setting a callback function, data receiption is done via event loop.
			/// if setting an empty callback function DataCb_t(), the event is taken out of the eventloop.
			/// \param dataCb callback to be called if fd gets readable (data is available)
			void setDataCb(DataCb_t dataCb);

			/// \param dataCb callback to be called if fd gets writable
			void setOutDataCb(DataCb_t dataCb);

			/// remove input event from eventloop
			void clearDataCb();
			/// remove output event from eventloop
			void clearOutDataCb();

			/// send everything or until connection closes
			/// uses gather mechanism to send several memory areas
			/// \warning waits until requested amount of data is processed or an error happened, hence it might block the eventloop if called from within a callback function
			ssize_t sendBlocks(const dataBlocks_t& blocks, bool more=0);

			/// send everything or until connection closes
			/// uses gather mechanism to send several memory areas
			/// \warning waits until requested amount of data is processed or an error happened, hence it might block the eventloop if called from within a callback function
			ssize_t sendBlocks(dataBlock_t* blocks, size_t blockCount, bool more=0);

			/// send everything or until connection closes
			/// \warning waits until requested amount of data is processed or an error happened, hence it might block the eventloop if called from within a callback function
			ssize_t sendBlock(const void* pBlock, size_t len, bool more);

			/// works as posix send
			ssize_t send(const void* pBlock, size_t len, bool more);

			/// might return with less bytes the requested
			ssize_t receive(void* pBlock, size_t len);

			/// might return with less bytes then requested if connection is being closed before completion
			/// \warning waits until requested amount of data is processed or an error happened, hence it might block the eventloop if called from within a callback function
			/// @param pBlock Receive buffer
			/// @param len Lenght of receive buffer
			/// @param msTimeout -1 for infinite
			ssize_t receiveComplete(void* pBlock, size_t len, int msTimeout = -1);

			/// \return true if socket uses firewire connection
			bool isFirewire() const;

			/// @param pCheckSockAddr The structure to compare the socket of this object with
			/// @param checkSockAddrLen Length of the structure depends on the type of socket (ipv4, ipv6)
			/// \return true if the socket of this object corresponds to the given sockaddr structure
			bool checkSockAddr(const struct sockaddr* pCheckSockAddr, socklen_t checkSockAddrLen) const;

			/// \return the file descriptor (Linux) or handle (Microsoft Windows)
			sys::event getEvent() const
			{
				return m_event;
			}

private:

			int setSocketOptions();

#ifdef _WIN32
			int process();
#endif

			sys::event m_event;

			BufferedReader m_bufferedReader;

			sys::EventLoop& m_eventLoop;
			DataCb_t m_inDataHandler;
#ifndef _WIN32
			DataCb_t m_outDataHandler;
#endif
		};
		
#ifdef _MSC_VER
		using clientSocket_t = std::shared_ptr <SocketNonblocking >;
#else
		using clientSocket_t = std::unique_ptr <SocketNonblocking >;
#endif
	}
}
#endif
