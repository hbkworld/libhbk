cmake_minimum_required(VERSION 3.13)

project(hbk_string_test LANGUAGES CXX)


enable_testing()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  SET(CMAKE_CXX_FLAGS "-O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls  -ftest-coverage -fprofile-arcs ${CMAKE_CXX_FLAGS}")
  SET(LDFLAGS "-fprofile-arcs -ftest-coverage ${LDFLAGS}")
endif()

add_executable( splittest splittest.cpp )
add_executable( replacetest replacetest.cpp )
add_executable( trimtest trimtest.cpp )
add_executable( readlinefromfiletest readlinefromfiletest.cpp )

get_property(targets DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach(tgt ${targets})
  get_target_property(target_type ${tgt} TYPE)
  if (target_type STREQUAL "EXECUTABLE")
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
      target_link_libraries(${tgt} ${ASAN_LIB} ${GCOV_LIB})
    endif()
    target_link_libraries(${tgt} ${CMAKE_THREAD_LIBS_INIT} testlib GTest::gtest GTest::gtest_main)

    set_target_properties(${tgt} PROPERTIES
      LINK_FLAGS "${EXTRA_LINK_FLAGS}"
      CXX_STANDARD_REQUIRED ON
      CXX_STANDARD 14
      CXX_EXTENSIONS OFF

    )
    add_test(NAME hbk.string.${tgt} COMMAND ${tgt}
      --report_level=no
      --log_level=all
    )
  endif()
endforeach()

