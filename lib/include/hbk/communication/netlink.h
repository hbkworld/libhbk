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

#ifndef _NETLINK_H
#define _NETLINK_H

#include <functional>

#include "hbk/exception/exception.hpp"
#include "hbk/communication/netadapterlist.h"
#include "hbk/sys/defines.h"
#include "hbk/sys/eventloop.h"

namespace hbk {
	namespace communication {
		/// Notifies about changes of network interfaces. This includes appearance/disappearance of interfaces and addresses
		class Netlink {
		public:
			/// types of netlink event
			enum event_t {
				/// interface got started
				LINK_ADDED,
				/// interface went down
				LINK_REMOVED,
				/// address was added to interface
				/// not supported under Windows
				ADDRESS_ADDED,
				/// address was removed from interface
				/// not supported under Windows
				ADDRESS_REMOVED,
				/// happens on initial start.
				/// windows version does not tell what happened it always sends COMPLETE to tell that something has happened.
				COMPLETE
			};

			/// type of callback function to execute to notify netlink event
			using interfaceAddressCb_t = std::function < void(event_t event, unsigned int adapterIndex, const std::string& ipv4Address) >;

			/// \throws hbk::exception
			Netlink(communication::NetadapterList &netadapterlist, sys::EventLoop &eventLoop);
			virtual ~Netlink();

			/// on execution of start, the callback function is being called with event = COMPLETE.
			int start(interfaceAddressCb_t interfaceAddressEventHandler);

			/// Remove this object from the event loop and close the netlink socket
			int stop();

		private:
			Netlink(const Netlink&);
			Netlink& operator= (const Netlink&);
			ssize_t process();

	#ifdef _WIN32
			int orderNextEvent();
	#else
			ssize_t receive(void *pReadBuffer, size_t bufferSize) const;

			/// receive events from netlink. Adapt netadapter list and mulicast server accordingly
			/// \param[in, out] netadapterlist will be adapted when processing netlink events
			/// \param[in, out] mcs will be adapted when processing netlink events
			void processNetlinkTelegram(void *pReadBuffer, size_t bufferSize) const;
	#endif
			sys::event m_event;
			
			communication::NetadapterList &m_netadapterlist;

			sys::EventLoop& m_eventloop;
			interfaceAddressCb_t m_interfaceAddressEventHandler;
		};
	}
}
#endif
