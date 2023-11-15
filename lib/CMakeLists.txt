cmake_minimum_required(VERSION 3.13)


# compiler warning levels are set by toolchain file

include(GNUInstallDirs)

set( HBKLIB_INTERFACE_HEADERS
    include/hbk/communication/ipv4address.h
    include/hbk/communication/ipv6address.h
    include/hbk/communication/bufferedreader.h
    include/hbk/communication/multicastserver.h
    include/hbk/communication/netadapter.h
    include/hbk/communication/netadapterlist.h
    include/hbk/communication/netlink.h
    include/hbk/communication/socketnonblocking.h
    include/hbk/communication/tcpserver.h
    include/hbk/debug/stack_trace.hpp
    include/hbk/exception/errno_exception.hpp
    include/hbk/exception/exception.hpp
    include/hbk/exception/jsonrpc_exception.h
    include/hbk/jsonrpc/jsonrpc_defines.h
    include/hbk/string/replace.h
    include/hbk/string/trim.h
    include/hbk/string/split.h
    include/hbk/string/readlinefromfile.h
    include/hbk/sys/defines.h
    include/hbk/sys/eventloop.h
    include/hbk/sys/executecommand.h
    include/hbk/sys/notifier.h
    include/hbk/sys/pidfile.h
    include/hbk/sys/timeconvert.h
    include/hbk/sys/timer.h
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set (PLATFORM_PATH "windows")
    message(STATUS "system name '${CMAKE_SYSTEM_NAME}'")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set (PLATFORM_PATH "linux")
    message(STATUS "system name '${CMAKE_SYSTEM_NAME}'")
else()
    message(FATAL_ERROR "unsupported system '${CMAKE_SYSTEM_NAME}'")
endif()


set(HBKLIB_SOURCES
  ${HBKLIB_INTERFACE_HEADERS}
  communication/ipv4address.cpp
  communication/ipv6address.cpp
  communication/${PLATFORM_PATH}/bufferedreader.cpp
  communication/${PLATFORM_PATH}/multicastserver.cpp
  communication/${PLATFORM_PATH}/netadapter.cpp
  communication/${PLATFORM_PATH}/netadapterlist.cpp
  communication/${PLATFORM_PATH}/netlink.cpp
  communication/${PLATFORM_PATH}/socketnonblocking.cpp
  communication/${PLATFORM_PATH}/tcpserver.cpp
  exception/errno_exception.cpp
  exception/exception.cpp
  exception/jsonrpc_exception.cpp
  string/replace.cpp
  string/split.cpp
  string/trim.cpp
  string/readlinefromfile.cpp
  sys/${PLATFORM_PATH}/eventloop.cpp
  sys/${PLATFORM_PATH}/executecommand.cpp
  sys/${PLATFORM_PATH}/notifier.cpp
  sys/${PLATFORM_PATH}/timer.cpp
  sys/pidfile.cpp
  sys/timeconvert.cpp
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(HBKLIB_SOURCES
    ${HBKLIB_SOURCES}
    communication/${PLATFORM_PATH}/wmi.cpp
)
endif()

add_library(${PROJECT_NAME} ${HBKLIB_SOURCES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(HBK_HARDWARE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -D_HBK_HARDWARE)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC -D_STANDARD_HARDWARE)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(${PROJECT_NAME} PUBLIC Ws2_32 Iphlpapi)
endif()


target_include_directories(${PROJECT_NAME} PUBLIC
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# see below!
#set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY PUBLIC_HEADER
#  ${HBKLIB_INTERFACE_HEADERS}
#)



set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
  SOVERSION ${PROJECT_VERSION_MAJOR}
  CXX_STANDARD_REQUIRED ON
  CXX_STANDARD 11
  CXX_EXTENSIONS OFF
# This one copies all files flat into the destination directory. Directory hierarchy is not kept!
#  PUBLIC_HEADER "${HBKLIB_INTERFACE_HEADERS}"
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY "include/${PROJECT_NAME}"
  DESTINATION include
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${${PROJECT_NAME}_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})


# Create and install stuff for find_package
install(
    EXPORT ${PROJECT_NAME}
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    FILE ${PROJECT_NAME}Config.cmake
)
