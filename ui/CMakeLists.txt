cmake_minimum_required(VERSION 3.8.0)
set(app_name ui)

find_package(SDL2)

set(hdr
    imgui.h
    imgui_internal.h
    imgui_impl_sdl.h
    imgui_impl_vulkan.h

    imgui_dlg.h
)

set(src
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_demo.cpp

    imgui_impl_sdl.cpp
    imgui_impl_vulkan.cpp

    imgui_dlg.cpp
)

add_definitions(-DUI_EXPORTS)

add_library(${app_name} SHARED ${src} ${hdr})


target_include_directories(${app_name} PRIVATE ${Vulkan_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR}/../Third-Party/Include)
target_link_directories(${app_name} PRIVATE ${CMAKE_BINARY_DIR}/lib ${Vulkan_INCLUDE_DIR}/../Third-Party/Bin)

if(MSVC)
target_link_libraries(${app_name} ${Vulkan_LIBRARY} SDL2.lib SDL2main.lib)
endif()
