cmake_minimum_required(VERSION 3.8.0)

project(aria)

if(CMAKE_CONFIGURATION_TYPES)
	#msvc etc
	foreach(CONF ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER "${CONF}" CONF)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF} ${CMAKE_BINARY_DIR}/lib)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF} ${CMAKE_BINARY_DIR}/bin)
	endforeach()
	set(CMAKE_DEBUG_POSTFIX "d")
else()
	#makefile ninja etc
	string(TOUPPER "${CMAKE_BUILD_TYPE}" CONF)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF} ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF} ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF} ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_DEBUG_POSTFIX "d")
endif()

add_subdirectory(aria)
add_subdirectory(libaria)

