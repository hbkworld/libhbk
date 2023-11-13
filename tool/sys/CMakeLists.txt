cmake_minimum_required(VERSION 3.13)

project(hbk_sys_tool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    SET(EXTRA_CXX_FLAGS "-fno-common -Wall -Werror -Wshadow -Wextra -Winit-self -Wstrict-overflow=5 -Wunused-result -Wcast-qual -Wcast-align -Wpointer-arith -Wformat=2 -Wwrite-strings -Wmissing-declarations -pedantic ${CMAKE_CXX_FLAGS}")
endif()


SET(EVENTLOOPPERFORMANCE_SOURCE
    eventloopperformance.cpp
)

add_executable(
    eventloopperformance
    ${EVENTLOOPPERFORMANCE_SOURCE}
)

target_link_libraries( eventloopperformance
    hbk
)

install(TARGETS eventloopperformance RUNTIME DESTINATION bin)
