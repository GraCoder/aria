#ifndef _UI_IMDLG_H_
#define _UI_IMDLG_H_

#include "common.h"

#include <vector>
#include <memory>
#include <functional>

struct SDL_Window;
struct ImGui_ImplVulkanH_Window;
struct ImDrawData;

class UI_EXPORT ImGuiDlg {
public:
	ImGuiDlg();
	~ImGuiDlg();

	void setRenderObject(const std::function<void(int, int)> &);
public:
	void				exec();
	VkInstance			instance();
	SDL_Window*			window();
	uint32_t			queueFamily();
	VkPhysicalDevice	physicalDevice();

	void cleanupVulkan();
	void setupVulkanWindow(int width, int height);
	void cleanupVulkanWindow();
private:
	void createInstance();
	void createSurface();
	void pickGpu();
	void selQueueFamily();
	void createDevice();

	void createDescriptorPool();
	void test();

	void initImGui();

	void FrameRender(ImDrawData* draw_data);
	void FramePresent();

	VkInstance			_instance = VK_NULL_HANDLE;
	SDL_Window*			_window = nullptr;
	VkPhysicalDevice	_physicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR		_surface;
	uint32_t			_queueFamily = -1;
	VkDevice            _device = VK_NULL_HANDLE;
	VkQueue             _queue = VK_NULL_HANDLE;
	VkPipelineCache     _pipelineCache = VK_NULL_HANDLE;
	VkDescriptorPool    _descriptorPool = VK_NULL_HANDLE;

	bool _swapChainRebuild = false;
	std::unique_ptr<ImGui_ImplVulkanH_Window> _vulkanWindow;

	VkAllocationCallbacks* _allocator = NULL;
	VkDebugReportCallbackEXT _debugReport = VK_NULL_HANDLE;
	std::vector<const char*> _extensions;

	std::function<void(int, int)> _renderObject;
};


#endif