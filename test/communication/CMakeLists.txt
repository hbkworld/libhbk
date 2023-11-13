cmake_minimum_required(VERSION 3.13)
project(hbk_communication_test LANGUAGES CXX)


enable_testing()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  SET(CMAKE_CXX_FLAGS "-O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls  -ftest-coverage -fprofile-arcs ${CMAKE_CXX_FLAGS}")
  SET(LDFLAGS "-fprofile-arcs -ftest-coverage ${LDFLAGS}")
endif()

find_package(Threads REQUIRED)

add_executable(
    multicastserver.test
    multicastserver_test.cpp
)

add_executable(
    socketnonblocking.test
    socketnonblocking_test.cpp
)

add_executable(
    netadapter.test
    netadapter_test.cpp
)

add_executable(
    ipv4address.test
    ipv4address_test.cpp
)

add_executable(
    ipv6address.test
    ipv6address_test.cpp
)



get_property(targets DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach(tgt ${targets})
  get_target_property(target_type ${tgt} TYPE)
  if (target_type STREQUAL "EXECUTABLE")
    target_link_libraries(${tgt} ${CMAKE_THREAD_LIBS_INIT} testlib GTest::gtest GTest::gtest_main) 
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries(${tgt} gcov)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        target_link_libraries(${tgt} Ws2_32)
    endif()

    set_target_properties(${tgt} PROPERTIES
      LINK_FLAGS "${EXTRA_LINK_FLAGS}"
      CXX_STANDARD_REQUIRED ON
      CXX_STANDARD 14
      CXX_EXTENSIONS OFF
    )
    add_test(NAME hbk.communication.${tgt} COMMAND ${tgt}
      --report_level=no
      --log_level=all
    )
  endif()
endforeach()

