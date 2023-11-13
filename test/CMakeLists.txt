cmake_minimum_required(VERSION 3.13)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/v1.12.0.zip
)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CGOV_COMPILE_FLAGS -fno-omit-frame-pointer -fno-optimize-sibling-calls -ftest-coverage -fprofile-arcs)
  set(GCOV_LINK_FLAGS -fprofile-arcs -ftest-coverage)
  set(GCOV_LIB gcov)
endif()

add_custom_target(coverage
  COMMAND echo "=================== Run ===================="
  COMMAND ${CMAKE_MAKE_PROGRAM} test ARGS="--output-on-failure"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(coverage
  PROPERTIES
  ADDITIONAL_CLEAN_FILES "${CMAKE_BINARY_DIR}/*.html"
)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES coverage)


#enable_testing()

# Those are the sources that are to be tested!
add_library(testlib OBJECT
    ../lib/communication/ipv4address.cpp
    ../lib/communication/ipv6address.cpp 
    ../lib/communication/linux/bufferedreader.cpp
    ../lib/communication/linux/multicastserver.cpp
    ../lib/communication/linux/netadapter.cpp
    ../lib/communication/linux/netadapterlist.cpp
    ../lib/communication/linux/netlink.cpp
    ../lib/communication/linux/socketnonblocking.cpp
    ../lib/communication/linux/tcpserver.cpp
    ../lib/exception/exception.cpp
    ../lib/exception/jsonrpc_exception.cpp
    ../lib/exception/errno_exception.cpp
    ../lib/string/replace.cpp
    ../lib/string/split.cpp
    ../lib/string/trim.cpp
    ../lib/string/readlinefromfile.cpp
    ../lib/sys/timeconvert.cpp
    ../lib/sys/pidfile.cpp
    ../lib/sys/linux/notifier.cpp
    ../lib/sys/linux/timer.cpp
    ../lib/sys/linux/eventloop.cpp
    ../lib/sys/linux/executecommand.cpp
)

target_link_libraries(testlib PUBLIC hbk::hbk)

target_compile_options(testlib PRIVATE
 $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
      -Wall -Werror ${CGOV_COMPILE_FLAGS} ${ASAN_COMPILE_FLAGS}>
)
target_link_options(testlib PRIVATE ${GCOV_LINK_FLAGS} )


add_subdirectory("communication")
add_subdirectory("string")
add_subdirectory("sys")



set(COMMON_BRANCH_OPTIONS "--exclude-unreachable-branches" "--exclude-throw-branches")
# exclude tests and external library code form coverage
# note: cmake replaces ' ' in string with '\ ' creating a list solves this problem; add --branches to use branch coverage again
set(EXCLUDED_DIRECTORIES "-e${CMAKE_SOURCE_DIR}/test" "-e${CMAKE_BINARY_DIR}/_deps")



add_custom_command(TARGET coverage
  COMMAND gcovr -r ${CMAKE_SOURCE_DIR} ${EXCLUDED_DIRECTORIES} --object-dir=${CMAKE_BINARY_DIR} ${COMMON_BRANCH_OPTIONS} --html-details -o ${CMAKE_BINARY_DIR}/coverage.html
  COMMAND gcovr -r ${CMAKE_SOURCE_DIR} ${EXCLUDED_DIRECTORIES} --object-dir=${CMAKE_BINARY_DIR} ${COMMON_BRANCH_OPTIONS} --json -o ${CMAKE_BINARY_DIR}/coverage.json
  COMMAND gcovr -r ${CMAKE_SOURCE_DIR} ${EXCLUDED_DIRECTORIES} --object-dir=${CMAKE_BINARY_DIR} ${COMMON_BRANCH_OPTIONS}
)
