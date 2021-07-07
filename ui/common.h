#ifndef _UI_APP_H_
#define _UI_APP_H_


#ifdef UI_EXPORTS
#define UI_EXPORT __declspec(dllexport)
#else
#define UI_EXPORT __declspec(dllimport)
#endif

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#else
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>

#endif