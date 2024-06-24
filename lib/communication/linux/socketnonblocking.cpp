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

#include <cerrno>
#include <cstring>
#include <cstdint>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include <unistd.h>
#include <fcntl.h>
#include <vector>

#include <syslog.h>
#include <poll.h>

#include "hbk/communication/socketnonblocking.h"

/// Maximum time to wait for connecting
constexpr time_t TIMEOUT_CONNECT_S = 5;


//#define WRITEV_TEST
#ifdef WRITEV_TEST
static const size_t MAX_BYTES_TO_PROCESS = 7000;

/// for testing reasons, we want to send a part only. The rest is to be processed afterwards!
static ssize_t sendmsgFake(hbk::communication::SocketNonblocking& instance, const struct msghdr *msg, int flags)
{

	size_t remaining = MAX_BYTES_TO_PROCESS;
	size_t bytesToSend;
	ssize_t bytesSend = 0;

	for (size_t blockIndex = 0; blockIndex<msg->msg_iovlen; ++blockIndex) {
		size_t blockSize = msg->msg_iov[blockIndex].iov_len;
		if (blockSize<remaining) {
			bytesToSend = blockSize;
		} else {
			bytesToSend = remaining;
		}
		ssize_t result = instance.sendBlock(msg->msg_iov[blockIndex].iov_base, bytesToSend, flags);
		if (result<=0) {
			return result;
		}
		bytesSend += result;
		remaining -= static_cast < size_t > (result);
		if (remaining==0) {
			return bytesSend;
		}
	}
	return bytesSend;
}
#endif


static int waitForWritable(int fd, int timeoutMilliSeconds)
{
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLOUT;

	int err;
	// wait for socket to become writable.
	do {
		err = poll(&pfd, 1, timeoutMilliSeconds);
	} while((err==-1) && (errno==EINTR));

	return err;
}

hbk::communication::SocketNonblocking::SocketNonblocking(sys::EventLoop &eventLoop)
	: m_event(-1)
	, m_bufferedReader()
	, m_eventLoop(eventLoop)
{
}

hbk::communication::SocketNonblocking::SocketNonblocking(int fd, sys::EventLoop &eventLoop)
	: m_event(fd)
	, m_bufferedReader()
	, m_eventLoop(eventLoop)
{
	if (m_event==-1) {
		throw std::runtime_error("not a valid socket");
	}
	if (fcntl(m_event, F_SETFL, O_NONBLOCK)==-1) {
		throw std::runtime_error("error setting socket to non-blocking");
	}
	if (setSocketOptions()<0) {
		throw std::runtime_error("error setting socket options");
	}
}

hbk::communication::SocketNonblocking::~SocketNonblocking()
{
	disconnect();
}

void hbk::communication::SocketNonblocking::setDataCb(DataCb_t dataCb)
{
	m_inDataHandler = dataCb;
	m_eventLoop.addEvent(m_event, std::bind(dataCb, std::ref(*this)));
}

void hbk::communication::SocketNonblocking::clearDataCb()
{
	m_inDataHandler = DataCb_t();
	m_eventLoop.eraseEvent(m_event);
}

void hbk::communication::SocketNonblocking::setOutDataCb(DataCb_t dataCb)
{
	m_outDataHandler = dataCb;
	m_eventLoop.addOutEvent(m_event, std::bind(dataCb, std::ref(*this)));
}

void hbk::communication::SocketNonblocking::clearOutDataCb()
{
	m_inDataHandler = DataCb_t();
	m_eventLoop.eraseOutEvent(m_event);
}

int hbk::communication::SocketNonblocking::setSocketOptions()
{
	int opt = 1;

	struct sockaddr_storage sockAddr;
	socklen_t sockAddrSize = sizeof(sockAddr);
	if (getsockname(m_event, reinterpret_cast< struct sockaddr * > (&sockAddr), &sockAddrSize) < 0) {
		syslog(LOG_ERR, "could not determine socket domain %s", strerror(errno));
		return -1;
	}
	if ((sockAddr.ss_family == AF_INET) || (sockAddr.ss_family == AF_INET6)) {
		// those are relevant for ip sockets only:

		// turn off Nagle algorithm
		if (setsockopt(m_event, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
			syslog(LOG_ERR, "error turning off nagle algorithm %s", strerror(errno));
			return -1;
		}

		opt = 12;
		// the interval between the last data packet sent (simple ACKs are not considered data) and the first keepalive probe;
		// after the connection is marked to need keepalive, this counter is not used any further
		if (setsockopt(m_event, SOL_TCP, TCP_KEEPIDLE, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
			syslog(LOG_ERR, "error setting socket option TCP_KEEPIDLE");
			return -1;
	}


		opt = 3;
		// the interval between subsequential keepalive probes, regardless of what the connection has exchanged in the meantime
		if (setsockopt(m_event, SOL_TCP, TCP_KEEPINTVL, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
			syslog(LOG_ERR, "error setting socket option TCP_KEEPINTVL");
			return -1;
		}


		opt = 2;
		// the number of unacknowledged probes to send before considering the connection dead and notifying the application layer
		if (setsockopt(m_event, SOL_TCP, TCP_KEEPCNT, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
			syslog(LOG_ERR, "error setting socket option TCP_KEEPCNT");
			return -1;
		}
	}


	opt = 1;
	if (setsockopt(m_event, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&opt), sizeof(opt))==-1) {
		syslog(LOG_ERR, "error setting socket option SO_KEEPALIVE");
		return -1;
	}

	return 0;
}


int hbk::communication::SocketNonblocking::connect(const std::string &address, const std::string& port)
{
	// tcp
	struct addrinfo hints;
	struct addrinfo* pResult = nullptr;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 6; // Ip V6!

	if( getaddrinfo(address.c_str(), port.c_str(), &hints, &pResult)!=0 ) {
		syslog(LOG_ERR, "could not get address information from '%s:%s': '%s'", address.c_str(), port.c_str(), strerror(errno));
		return -1;
	}
	int retVal = connect(pResult->ai_family, pResult->ai_addr, pResult->ai_addrlen);
	if (retVal < 0) {
		syslog(LOG_ERR, "could not connect to tcp socket: '%s'", strerror(errno));
	}
	freeaddrinfo( pResult );
	return retVal;
}

int hbk::communication::SocketNonblocking::connect(const std::string &path, bool useAbstractNamespace)
{
	struct sockaddr_un sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sun_family = AF_UNIX;

	socklen_t serveraddrLen;
	if (useAbstractNamespace) {
		/* unix domain socket!
		 * abstract socket address is distinguished by putting '\0' in the first byte of the address
		 * addrlen for bind has to be calculated! This includes the address prefix '\0'
		*/
		serveraddrLen = static_cast < socklen_t > (1+path.length()+sizeof(sockaddr.sun_family));
		strncpy(&sockaddr.sun_path[1], path.c_str(), sizeof(sockaddr.sun_path)-2);
	} else {
		serveraddrLen = static_cast < socklen_t > (path.length()+sizeof(sockaddr.sun_family));
		strncpy(sockaddr.sun_path, path.c_str(), sizeof(sockaddr.sun_path)-1);
	}

	int retVal = connect(AF_UNIX, reinterpret_cast < struct sockaddr* > (&sockaddr), serveraddrLen);
	if (retVal < 0) {
		syslog(LOG_ERR, "could not connect to unix domain socket '%s': '%s'", path.c_str(), strerror(errno));
	}
	return retVal;
}

int hbk::communication::SocketNonblocking::connect(int domain, const struct sockaddr* pSockAddr, socklen_t len)
{
	m_event = ::socket(domain, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (m_event==-1) {
		return -1;
	}

	// callback functions might have already been set before fd was created.
	if (m_inDataHandler) {
		m_eventLoop.addEvent(m_event, std::bind(m_inDataHandler, std::ref(*this)));
	}

	if (m_outDataHandler) {
		m_eventLoop.addOutEvent(m_event, std::bind(m_outDataHandler, std::ref(*this)));
	}

	if (setSocketOptions()<0) {
		return -1;
	}

	int err = ::connect(m_event, pSockAddr, len);
	if (err==-1) {
		// success if errno equals EINPROGRESS
		if(errno != EINPROGRESS) {
			syslog(LOG_ERR, "failed to connect socket (errno=%d '%s')", errno, strerror(errno));
			return -1;
		}
		err = waitForWritable(m_event, TIMEOUT_CONNECT_S*1000);
		if (err!=1) {
			return -1;
		}

		int value;
		len = sizeof(value);
		getsockopt(m_event, SOL_SOCKET, SO_ERROR, &value, &len);
		if(value!=0) {
			return -1;
		}
	}
	return 0;
}

ssize_t hbk::communication::SocketNonblocking::receive(void* pBlock, size_t size)
{
	return m_bufferedReader.recv(m_event, pBlock, size);
}

ssize_t hbk::communication::SocketNonblocking::receiveComplete(void* pBlock, size_t size, int msTimeout)
{
	ssize_t retVal;
	unsigned char* pPos = reinterpret_cast < unsigned char* > (pBlock);
	size_t sizeLeft = size;
	while (sizeLeft) {
		retVal = m_bufferedReader.recv(m_event, pPos, sizeLeft);
		if (retVal>0) {
			sizeLeft -= static_cast < size_t > (retVal);
			pPos += retVal;
		} else if (retVal==0) {
			return static_cast < int > (size-sizeLeft);
		} else {
			if(errno==EWOULDBLOCK || errno==EAGAIN) {
				// wait for socket to become readable.
				struct pollfd pfd;
				pfd.fd = m_event;
				pfd.events = POLLIN;
				int nfds;
				do {
					nfds = poll(&pfd, 1, msTimeout);
				} while((nfds==-1) && (errno==EINTR));
				if(nfds!=1) {
					return -1;
				}
			} else {
				syslog(LOG_ERR, "%s: recv failed '%s'", __FUNCTION__, strerror(errno));
				return -1;
			}
		}
	}
	return static_cast < ssize_t > (size);
}


ssize_t hbk::communication::SocketNonblocking::sendBlocks(dataBlock_t *blocks, size_t blockCount, bool more)
{
	msghdr msgHdr;
	memset(&msgHdr, 0, sizeof(msgHdr));

	size_t totalLength = 0;
	size_t totalBytesRemaining;

	int flags = MSG_NOSIGNAL;
	if (more) {
		flags = MSG_MORE;
	}
	for(size_t blockIndex = 0; blockIndex<blockCount; ++blockIndex) {
		totalLength += blocks[blockIndex].size;
	}
	totalBytesRemaining = totalLength;
	
	dataBlock_t *pBlockPos = blocks;
	
	ssize_t retVal;
	size_t bytesWritten;
	do {
		// we use sendmsg instead of writev because we want to set the flag parameter
		msgHdr.msg_iov = const_cast < iovec * > (reinterpret_cast < const iovec * > (pBlockPos));
		msgHdr.msg_iovlen = blockCount;
#ifdef WRITEV_TEST
		retVal = sendmsgFake(*this, &msgHdr, flags);
#else
		retVal = sendmsg(m_event, &msgHdr, flags);
#endif
		if (retVal==0) {
			return retVal;
		} else if (retVal==-1) {
			if ((errno!=EWOULDBLOCK) && (errno!=EAGAIN) && (errno!=EINTR) ) {
				syslog (LOG_ERR, "writev() failed: '%s'", strerror(errno));
				return retVal;
			}
			bytesWritten = 0;
		} else {
			bytesWritten = static_cast < size_t >(retVal);
		}
		
		totalBytesRemaining -= bytesWritten;
		if (totalBytesRemaining==0) {
			// we are done!
			return static_cast < ssize_t > (totalLength);
		}
		// in this case we might have written nothing at all or only a part
		// reorganize buffer and write again...
		size_t remainingOffset = bytesWritten;
		do {
			if (remainingOffset>=pBlockPos->size) {
				remainingOffset -= pBlockPos->size;
				pBlockPos++;
				blockCount--;
			} else {
				pBlockPos->size -= remainingOffset;
				pBlockPos->pData = reinterpret_cast < const uint8_t* > (pBlockPos->pData) + remainingOffset;
				break;
			}
		} while (true);
		
		//syslog(LOG_INFO, "%zu bytes in %zu blocks left", totalBytesRemaining, blockCount);
		retVal = waitForWritable(m_event, -1);
		if (retVal!=1) {
			return -1;
		}
	} while(totalBytesRemaining);
	return static_cast < ssize_t > (totalLength);
}



ssize_t hbk::communication::SocketNonblocking::sendBlocks(const dataBlocks_t &blocks, bool more)
{
	size_t size = blocks.size();
	std::vector < dataBlock_t > dataBlockVector;
	dataBlockVector.reserve(size);
	std::copy(blocks.cbegin(), blocks.cend(), std::back_inserter(dataBlockVector));

	return sendBlocks(dataBlockVector.data(), size, more);
}

ssize_t hbk::communication::SocketNonblocking::sendBlock(const void* pBlock, size_t size, bool more)
{
	const uint8_t* pDat = reinterpret_cast<const uint8_t*>(pBlock);
	size_t BytesLeft = size;
	ssize_t numBytes;
	ssize_t retVal = static_cast < ssize_t > (size);

	int flags = MSG_NOSIGNAL;
	if(more) {
		flags |= MSG_MORE;
	}
	int err;

	while (BytesLeft > 0) {
		numBytes = ::send(m_event, pDat, BytesLeft, flags);
		if (numBytes>0) {
			pDat += numBytes;
			BytesLeft -= static_cast < size_t > (numBytes);
		} else if(numBytes==0) {
			// connection lost...
			BytesLeft = 0;
			retVal = -1;
		} else {
			// <0
			if(errno==EWOULDBLOCK || errno==EAGAIN) {
				// wait for socket to become writable.
				err = waitForWritable(m_event, -1);
				if (err!=1) {
					BytesLeft = 0;
					retVal = -1;
				}
			} else if (errno!=EINTR) {
				// a real error happened!
				BytesLeft = 0;
				retVal = -1;
			}
		}
	}
	return retVal;
}


ssize_t hbk::communication::SocketNonblocking::send(const void* pBlock, size_t len, bool more)
{
	int flags = MSG_NOSIGNAL;
	if(more) {
		flags |= MSG_MORE;
	}
	return ::send(m_event, pBlock, len, flags);
}


bool hbk::communication::SocketNonblocking::checkSockAddr(const struct ::sockaddr* pCheckSockAddr, socklen_t checkSockAddrLen) const
{
	struct sockaddr sockAddr;
	socklen_t addrLen = sizeof(sockaddr_in);

	char checkHost[256] = "";
	char ckeckService[8] = "";

	char host[256] = "";
	char service[8] = "";
	int err = getnameinfo(pCheckSockAddr, checkSockAddrLen, checkHost, sizeof(checkHost), ckeckService, sizeof(ckeckService), NI_NUMERICHOST | NI_NUMERICSERV);
	if (err != 0) {
		syslog(LOG_ERR, "%s: error from getnameinfo", __FUNCTION__);
		return false;
	}

	if (getpeername(m_event, &sockAddr, &addrLen)!=0) {
		return false;
	}

	if (getnameinfo(&sockAddr, addrLen, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV)!=0) {
		return false;
	}

	if ((strcmp(host, checkHost)==0) && (strcmp(service, ckeckService)==0)
		)
	{
		return true;
	}
	return false;
}

void hbk::communication::SocketNonblocking::disconnect()
{
	if (m_event!=-1) {
		if (::close(m_event)) {
			syslog(LOG_ERR, "closing socket %d failed '%s'", m_event, strerror(errno));
		}
		m_eventLoop.eraseEvent(m_event);
		m_eventLoop.eraseOutEvent(m_event);
	}

	m_event = -1;
}

bool hbk::communication::SocketNonblocking::isFirewire() const
{
	bool retVal = false;

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));

	if ((::ioctl(m_event, SIOCGIFHWADDR, &ifr, sizeof(struct ifreq))) >= 0) {
		if (ifr.ifr_hwaddr.sa_family == ARPHRD_IEEE1394) {
			retVal = true;
		}
	}
	return retVal;
}
