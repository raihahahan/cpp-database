file(GLOB_RECURSE SRC_HEADERS CONFIGURE_DEPENDS *.hpp)
file(GLOB_RECURSE SRC_SOURCES CONFIGURE_DEPENDS *.cpp)

add_library(db_core ${SRC_HEADERS} ${SRC_SOURCES})
target_include_directories(db_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(db_main main.cpp)
target_link_libraries(db_main PRIVATE db_core)
