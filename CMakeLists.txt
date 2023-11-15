cmake_minimum_required(VERSION 3.13)

project(hbk VERSION 1.0.71 LANGUAGES CXX)

option(HBK_GENERATE_DOC         "Generate documentation"                            OFF)
option(HBK_POST_BUILD_UNITTEST  "Automatically run unit-tests as a post build step" OFF)
option(HBK_TOOLS                "Compile tools" OFF)
option(HBK_HARDWARE             "Building for hardware target"                      ON)

add_subdirectory("lib")

if(HBK_GENERATE_DOC)
    include(cmake/generate_doxygen.cmake)
endif()

if(HBK_TOOLS)
    add_subdirectory("tool/communication")
    add_subdirectory("tool/sys")
endif()

if(HBK_POST_BUILD_UNITTEST)
    enable_testing()
    add_subdirectory("test")
endif()
