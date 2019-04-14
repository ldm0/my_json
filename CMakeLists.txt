cmake_minimum_required(VERSION 2.6)
project(my_json_parser_test C)

add_library(my_json_parser_lib my_json_parser.c)
add_library(my_json_writer_lib my_json_writer.c)
add_executable(my_json_test my_json_test.c)
target_link_libraries(my_json_test my_json_parser_lib my_json_writer_lib)