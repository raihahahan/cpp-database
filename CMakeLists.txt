option(ENABLE_TESTING "Build tests" ON)

cmake_minimum_required(VERSION 3.18)
project(cpp_database LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(ENABLE_TESTING)
    find_package(Catch2 3 REQUIRED)
    enable_testing()
    add_subdirectory(Catch2)
    add_subdirectory(tests)
endif()
add_executable(db_cli src/cli/main.cpp)
target_link_libraries(db_cli PRIVATE db_core)
add_subdirectory(src)

