cmake_minimum_required(VERSION 3.8.0)

project(aria)

find_package(Vulkan)
find_package(SDL2)

foreach(CONF ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER "${CONF}" CONF)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF} ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF} ${CMAKE_BINARY_DIR}/bin)
endforeach()

set(CMAKE_DEBUG_POSTFIX "d")

add_subdirectory(ui)
add_subdirectory(aria)
