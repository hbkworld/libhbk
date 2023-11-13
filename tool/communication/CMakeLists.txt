cmake_minimum_required(VERSION 3.13)

project(hbk_communication_tool LANGUAGES CXX)

add_executable(
  netadapterview
  netadapterview.cpp
)

add_executable(
    netlinkview
    netlinkview.cpp
)

add_executable(
    server
    Server.cpp
)

add_executable(
    client
    Client.cpp
)

get_property(targets DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach(tgt ${targets})
  get_target_property(target_type ${tgt} TYPE)
  if (target_type STREQUAL "EXECUTABLE")
    target_link_libraries(${tgt} hbk::hbk)

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        target_link_libraries(${tgt} Iphlpapi)
    endif()

    set_target_properties(${tgt} PROPERTIES
      CXX_STANDARD_REQUIRED ON
      CXX_STANDARD 11
      CXX_EXTENSIONS OFF
    )
  endif()
endforeach()
