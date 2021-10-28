cmake_minimum_required(VERSION 3.8.0)
set(lib_name aria2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB hdrs *.h)

file(GLOB srcs *.cc *.c)

find_package(libXml2)

add_library(${lib_name} SHARED ${srcs} ${hdrs})
target_compile_definitions(${lib_name} PUBLIC HAVE_CONFIG_H)

target_include_directories(${lib_name} PUBLIC ${LIBXML2_INCLUDE_DIR})
target_link_libraries(${lib_name} ${LIBXML2_LIBRARY} sqlite3 ssh2 cares)
target_link_libraries(${lib_name} gmp ssl z intl iconv)
target_link_libraries(${lib_name} crypt32 secur32 advapi32)
target_link_libraries(${lib_name} ws2_32 wsock32 gdi32 winmm psapi)

