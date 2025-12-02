cmake_minimum_required(VERSION 3.18)

project(hbk VERSION 2.2.3 LANGUAGES CXX)

option(HBK_GENERATE_DOC         "Generate documentation"                            OFF)
option(HBK_POST_BUILD_UNITTEST  "Automatically run unit-tests as a post build step" OFF)
option(HBK_TOOLS                "Compile tools"                                     OFF)
option(HBK_HARDWARE             "Building for hardware target"                      OFF)

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

set(CPACK_PACKAGE_CONTACT               "Hottinger Brüel & Kjær <info@hbkworld.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "HBK Utility Library")
set(CPACK_PACKAGE_DESCRIPTION           "This is a utility library used by other HBK libraries.")
set(CPACK_PACKAGE_DIRECTORY             "${CMAKE_CURRENT_BINARY_DIR}/packages")
set(CPACK_PACKAGE_NAME                  "libhbk")
set(CPACK_PACKAGE_VENDOR                "Hottinger Brüel & Kjær")
set(CPACK_PACKAGE_FILE_NAME             "${CPACK_PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}")

set(CPACK_DEBIAN_PACKAGE_RELEASE        1)
set(CPACK_DEBIAN_PACKAGE_DEPENDS        "libc6 (>= 2.14), libstdc++6 (>= 3.4.23)")
set(CPACK_DEBIAN_PACKAGE_SECTION        "libs")
set(CPACK_DEBIAN_FILE_NAME              "${CPACK_PACKAGE_NAME}_${${PROJECT_NAME}_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")

set(CPACK_RPM_PACKAGE_RELEASE           1)
set(CPACK_RPM_FILE_NAME                 "${CPACK_PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}.${CPACK_RPM_PACKAGE_ARCHITECTURE}.rpm")

include(CPack)
