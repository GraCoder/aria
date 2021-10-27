cmake_minimum_required(VERSION 3.8.0)
set(lib_name libaria)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB hdrs *.h)

file(GLOB srcs *.cc *.c)



find_package(libxml2)

add_library(${lib_name} ${srcs} ${hdrs})
target_compile_definitions(${lib_name} PUBLIC HAVE_CONFIG_H)

target_include_directories(${lib_name} PUBLIC ${LIBXML2_INCLUDE_DIR})
target_link_libraries(${lib_name} ${LIBXML2_LIBRARY})

