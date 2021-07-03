#include "imgui_app.h"
#include <stdio.h>

#include <SDL2/SDL.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL 
debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}

ImGuiApp::ImGuiApp()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
		printf("Error: %s\n", SDL_GetError());
		return;
	}

	createInstance();
	pickGpu();
	selQueueFamily();
}

void ImGuiApp::run()
{
	while (true) {

	};
}

void ImGuiApp::createInstance()
{
	// Create Vulkan Instance
	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.enabledExtensionCount = _extensions.size();
	create_info.ppEnabledExtensionNames = _extensions.data();
#ifdef IMGUI_VULKAN_DEBUG_REPORT
	// Enabling validation layers
	const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
	create_info.enabledLayerCount = 1;
	create_info.ppEnabledLayerNames = layers;

	_extensions.push_back("VK_EXT_debug_report");
	create_info.enabledExtensionCount = _extensions.size();
	create_info.ppEnabledExtensionNames = _extensions.data();

	auto err = vkCreateInstance(&create_info, _allocate, &_instance);

	auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");

	// Setup the debug report callback
	VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
	debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	debug_report_ci.pfnCallback = debug_report;
	debug_report_ci.pUserData = NULL;
	err = vkCreateDebugReportCallbackEXT(_instance, &debug_report_ci, _allocate, &_debugReport);
#else
	// Create Vulkan Instance without any debug feature
	err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
	check_vk_result(err);
#endif
}

void ImGuiApp::pickGpu()
{
	uint32_t gpu_count;
	auto err = vkEnumeratePhysicalDevices(_instance, &gpu_count, NULL);

	std::vector<VkPhysicalDevice> gpus(gpu_count);
	err = vkEnumeratePhysicalDevices(_instance, &gpu_count, gpus.data());
	
	int use_gpu = 0;
	for (int i = 0; i < (int)gpu_count; i++)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(gpus[i], &properties);
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			use_gpu = i;
			break;
		}
	}
	_physicalDevice = gpus[use_gpu];
}

void ImGuiApp::selQueueFamily()
{
	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, NULL);
	std::vector<VkQueueFamilyProperties> queues(count);
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, queues.data());
	for (uint32_t i = 0; i < count; i++)
		if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			_queueFamily = i;
			break;
		}
}

void ImGuiApp::test()
{
}
