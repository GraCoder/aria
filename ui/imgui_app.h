#ifndef _UI_IMAPP_H_
#define _UI_IMAPP_H_

#include "common.h"

#include "vulkan/vulkan.h"

#include <vector>

class UI_EXPORT ImGuiApp {
public:
	ImGuiApp();

	void run();

private:
	void createInstance();
	void pickGpu();
	void selQueueFamily();
	void test();

	VkInstance			_instance = VK_NULL_HANDLE;
	VkPhysicalDevice    _physicalDevice = VK_NULL_HANDLE;
	uint32_t			_queueFamily = -1;

	VkAllocationCallbacks *_allocate = NULL;
	VkDebugReportCallbackEXT _debugReport = VK_NULL_HANDLE;

	std::vector<char*> _extensions;
};


#endif