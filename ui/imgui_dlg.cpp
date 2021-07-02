#include "imgui_dlg.h"

#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>


ImGuiDlg::ImGuiDlg()
{
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+Vulkan example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

	//uint32_t extensions_count = 0;
	//SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, NULL);
	//const char** extensions = new const char* [extensions_count];
	//SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, extensions);
	//SetupVulkan(extensions, extensions_count);
	//delete[] extensions;

	//// Create Window Surface
	//VkSurfaceKHR surface;
	//VkResult err;
	//if (SDL_Vulkan_CreateSurface(window, g_Instance, &surface) == 0)
	//{
	//	printf("Failed to create Vulkan surface.\n");
	//}
}
