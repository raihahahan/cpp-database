cmake_minimum_required(VERSION 3.16)

project(tests)

find_package(Catch2 3 REQUIRED)
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp")
foreach(test_src ${TEST_SOURCES})
    message(STATUS "Test source added: ${test_src}")
endforeach()

add_executable(tests ${TEST_SOURCES})

target_compile_features(tests PUBLIC cxx_std_20)
target_include_directories(tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain db_core)

include(CTest)
include(Catch)
catch_discover_tests(tests)
