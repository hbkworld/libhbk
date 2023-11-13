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

#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#define ssize_t int
#include <chrono>
#include <mutex>

#include "hbk/sys/eventloop.h"

#ifdef UNICODE
#define LPSTRINGTYPE LPCWSTR
#else
#define LPSTRINGTYPE LPCSTR
#endif
namespace hbk {
	namespace sys {
		EventLoop::EventLoop()
			: m_completionPort(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1))
			, m_hEventLog(RegisterEventSource(nullptr, reinterpret_cast < LPSTRINGTYPE > ("Application")))
		{
		}

		EventLoop::~EventLoop()
		{
			stop();
			DeregisterEventSource(m_hEventLog);
			CloseHandle(m_completionPort);
		}

		int EventLoop::addEvent(event fd, const EventHandler_t &eventHandler)
		{
			if (!eventHandler) {
				return -1;
			}

			{
				std::lock_guard < std::recursive_mutex > lock(m_eventInfosMtx);
				m_eventInfos[fd.overlapped.hEvent] = eventHandler;
			}
			
			if (fd.fileHandle == INVALID_HANDLE_VALUE) {
				return 0;
			}

			// We use the completionkey to determine the event to handle.
			// Using the ovelapped structure is dangearous because it is owned by the object that registers the event.
			// After destruction, GetQueuedCompletionStatus() might deliver an overlapped structure that does not exist anymore!
			ULONG completionKey = (ULONG)fd.overlapped.hEvent;
			if (CreateIoCompletionPort(fd.fileHandle, m_completionPort, (ULONG_PTR)completionKey, 1) == nullptr) {
				int lastError = GetLastError();
				// ERROR_INVALID_PARAMETER means that this handle is already registered
				if (lastError != ERROR_INVALID_PARAMETER) {
					std::string message;
					LPCSTR messages;

					message = "Could not add event to event loop '" + std::to_string(lastError) + "'";
					messages = message.c_str();
					ReportEvent(m_hEventLog, EVENTLOG_ERROR_TYPE, 0, 0, nullptr, 1, 0, reinterpret_cast < LPSTRINGTYPE* > (&messages), nullptr);
					return -1;
				}
			}
			return 0;

		}

		int EventLoop::eraseEvent(event fd)
		{
			std::lock_guard < std::recursive_mutex > lock(m_eventInfosMtx);
			if (m_eventInfos.erase(fd.overlapped.hEvent) == 0) {
				return -1;
			}
			return 0;
		}

		int EventLoop::execute()
		{
			BOOL result;
			DWORD size;
			ULONG_PTR completionKey = 0;
			OVERLAPPED* pOverlapped;


			if (m_completionPort == nullptr) {
				LPCSTR messages = "Could not create io completion port!";
				ReportEvent(m_hEventLog, EVENTLOG_ERROR_TYPE, 0, 0, nullptr, 1, 0, reinterpret_cast < LPSTRINGTYPE* > (&messages), nullptr);
				return -1;
			}
			
			do {
				pOverlapped = nullptr;
				result = GetQueuedCompletionStatus(m_completionPort, &size, &completionKey, &pOverlapped, INFINITE);
				if (result == FALSE) {
					if (pOverlapped==nullptr) {
						// nothing was dequeued...
						LPCSTR messages = "Event loop woke up but nothing was dequeued. Waiting for next events...";
						ReportEvent(m_hEventLog, EVENTLOG_ERROR_TYPE, 0, 0, nullptr, 1, 0, reinterpret_cast < LPSTRINGTYPE* > (&messages), nullptr);
					} else {
						int lastError = GetLastError();
						
						if (lastError == ERROR_NETNAME_DELETED) {
							HANDLE event = (HANDLE)completionKey;
							//HANDLE event = pOverlapped->hEvent;

							// ERROR_NETNAME_DELETED happens on closure of connection
							// Happpens also if tcp keep alive recognizes loss of connection.
							// We need to call the callback routine only once to handle the error.
							std::lock_guard < std::recursive_mutex > lock(m_eventInfosMtx);
							eventInfos_t::iterator iter = m_eventInfos.find(event);
							if (iter != m_eventInfos.end()) {
								iter->second();
							}
						} else if (lastError != ERROR_OPERATION_ABORTED) {
							// ERROR_OPERATION_ABORTED happens on cancelation of an overlapped operation and is to be ignored.
							std::string message;
							LPCSTR messages;
	
							message = "Event loop stopped with error " + std::to_string(lastError);
							messages = message.c_str();
							ReportEvent(m_hEventLog, EVENTLOG_ERROR_TYPE, 0, 0, nullptr, 1, 0, reinterpret_cast < LPSTRINGTYPE* > (&messages), nullptr);
							break;
						}
					}
				} else {
//					if (pOverlapped==nullptr) {
//						// stop condition
//						break;
//					}

//					{
					ssize_t retval = 0;
					HANDLE event = (HANDLE)completionKey;
					//HANDLE event = pOverlapped->hEvent;

					if (event == WSA_INVALID_EVENT) {
						// stop condition
						return 0;
					}

					do {
						std::lock_guard < std::recursive_mutex > lock(m_eventInfosMtx);
						eventInfos_t::iterator iter = m_eventInfos.find(event);
						if (iter != m_eventInfos.end()) {
							retval = iter->second();
						}
					} while (retval > 0);
//					}
				}
			} while (true);
			return 0;
		}

		void EventLoop::stop()
		{
			PostQueuedCompletionStatus(m_completionPort, 0, (ULONG_PTR)WSA_INVALID_EVENT, nullptr);
		}
	}
}
