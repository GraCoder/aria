cmake_minimum_required(VERSION 3.8.0)
set(app_name aria)


set(src
	main.cpp
)


add_executable(${app_name} main.cpp)


target_include_directories(${app_name} PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(${app_name} PRIVATE ${CMAKE_SOURCE_DIR}/ui)

target_link_directories(${app_name} PRIVATE ${CMAKE_BINARY_DIR}/lib)

target_link_libraries(${app_name} ${Vulkan_LIBRARY})
target_link_libraries(${app_name} SDL2::SDL2)
target_link_libraries(${app_name} "uid")

set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${app_name}>"
                                               VS_DEBUGGER_COMMAND           "$<TARGET_FILE:${app_name}>"
                                               VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin")
