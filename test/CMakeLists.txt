cmake_minimum_required(VERSION 2.8.11)
project(my_json_test)

# googletest module
configure_file(gtest_cmake.txt googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(
    ${CMAKE_BINARY_DIR}/googletest-src
    ${CMAKE_BINARY_DIR}/googletest-build
    EXCLUDE_FROM_ALL)
# end googletest module

add_library(my_json_parser_lib ../src/my_json_parser.c)
add_library(my_json_writer_lib ../src/my_json_writer.c)
add_library(my_json_type_lib ../src/my_json_type.c)
add_executable(my_json_test my_json_test.cpp)
target_link_libraries(my_json_test gtest_main my_json_parser_lib my_json_writer_lib my_json_type_lib)