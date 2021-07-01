cmake_minimum_required(VERSION 3.8.0)
set(app_name ui)

set(hdr
    imgui.h
    imgui_internal.h
    imgui_impl_sdl.h
    imgui_impl_vulkan.h
)

set(src
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_demo.cpp

    imgui_impl_sdl.cpp
    imgui_impl_vulkan.cpp
)

add_library(${app_name} ${src} ${hdr})

target_include_directories(${app_name} PRIVATE ${Vulkan_INCLUDE_DIR})

target_link_libraries(${app_name} ${Vulkan_LIBRARY})
target_link_libraries(${app_name} SDL2::SDL2)
