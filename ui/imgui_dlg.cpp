#include <stdio.h>

#include "imgui.h"
#include "imgui_dlg.h"

#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

static const int MinImageCount = 2;

static VKAPI_ATTR VkBool32 VKAPI_CALL 
debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

ImGuiDlg::ImGuiDlg()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
		printf("Error: %s\n", SDL_GetError());
		return;
	}

	_window = SDL_CreateWindow("ARIA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	createInstance();
	createSurface();
	pickGpu();
	selQueueFamily();
	createDevice();
	createDescriptorPool();

	_vulkanWindow = std::make_unique<ImGui_ImplVulkanH_Window>();

	int w, h;
	SDL_GetWindowSize(_window, &w, &h);
	setupVulkanWindow(w, h);

	initImGui();
}

ImGuiDlg::~ImGuiDlg()
{
	auto err = vkDeviceWaitIdle(_device);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	cleanupVulkanWindow();
	cleanupVulkan();

	SDL_DestroyWindow(_window);
	SDL_Quit();
}

void ImGuiDlg::exec()
{
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	bool done = false;
	while (!done) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(_window))
				done = true;
		}

		if (_swapChainRebuild) {
			int width, height;
			SDL_GetWindowSize(_window, &width, &height);
			if (width > 0 && height > 0) {
				ImGui_ImplVulkan_SetMinImageCount(MinImageCount);
				ImGui_ImplVulkanH_CreateOrResizeWindow(_instance, _physicalDevice,
					_device, _vulkanWindow.get(), _queueFamily, _allocator, width, height, MinImageCount);
				_vulkanWindow->FrameIndex = 0;
				_swapChainRebuild = false;
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(_window);
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window) {
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		if (!is_minimized) {
			_vulkanWindow->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
			_vulkanWindow->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
			_vulkanWindow->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
			_vulkanWindow->ClearValue.color.float32[3] = clear_color.w;
			FrameRender(draw_data);
			FramePresent();
		}
	}
}

VkInstance ImGuiDlg::instance()
{
	return _instance;
}

SDL_Window* ImGuiDlg::window()
{
	return _window;
}

uint32_t ImGuiDlg::queueFamily()
{
	return _queueFamily;
}

VkPhysicalDevice ImGuiDlg::physicalDevice()
{
	return _physicalDevice;
}

void ImGuiDlg::cleanupVulkan()
{
	vkDestroyDescriptorPool(_device, _descriptorPool, _allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
	// Remove the debug report callback
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
	vkDestroyDebugReportCallbackEXT(_instance, _debugReport, _allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

	vkDestroyDevice(_device, _allocator);
	vkDestroyInstance(_instance, _allocator);
}

void ImGuiDlg::setupVulkanWindow(int width, int height)
{
	_vulkanWindow->Surface = _surface;

	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, _queueFamily, _surface, &res);
	if (res != VK_TRUE) {
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	_vulkanWindow->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
		_physicalDevice, _surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
	_vulkanWindow->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
		_physicalDevice, _surface, &present_modes[0], IM_ARRAYSIZE(present_modes));

	ImGui_ImplVulkanH_CreateOrResizeWindow(_instance,
		_physicalDevice, _device, _vulkanWindow.get(), _queueFamily, _allocator, width, height, 2);

}

void ImGuiDlg::cleanupVulkanWindow()
{
	ImGui_ImplVulkanH_DestroyWindow(_instance, _device, _vulkanWindow.get(), _allocator);
}

void ImGuiDlg::createInstance()
{
	uint32_t extensions_count = 0;
	SDL_Vulkan_GetInstanceExtensions(_window, &extensions_count, NULL);
	_extensions.resize(extensions_count);
	SDL_Vulkan_GetInstanceExtensions(_window, &extensions_count, _extensions.data());

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

	auto err = vkCreateInstance(&create_info, _allocator, &_instance);

	auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");

	// Setup the debug report callback
	VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
	debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	debug_report_ci.pfnCallback = debug_report;
	debug_report_ci.pUserData = NULL;
	err = vkCreateDebugReportCallbackEXT(_instance, &debug_report_ci, _allocator, &_debugReport);
#else
	// Create Vulkan Instance without any debug feature
	err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
	check_vk_result(err);
#endif
}

void ImGuiDlg::createSurface()
{
	if (SDL_Vulkan_CreateSurface(ImGuiDlg::window(), ImGuiDlg::instance(), &_surface) == 0) {
		char errMsg[2048];
		SDL_GetErrorMsg(errMsg, 2048);
		printf("Failed to create Vulkan surface.\n");
	}
}

void ImGuiDlg::pickGpu()
{
	uint32_t gpu_count;
	auto err = vkEnumeratePhysicalDevices(_instance, &gpu_count, NULL);

	std::vector<VkPhysicalDevice> gpus(gpu_count);
	err = vkEnumeratePhysicalDevices(_instance, &gpu_count, gpus.data());

	int use_gpu = 0;
	for (int i = 0; i < (int)gpu_count; i++) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(gpus[i], &properties);
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			use_gpu = i;
			break;
		}
	}
	_physicalDevice = gpus[use_gpu];
}

void ImGuiDlg::selQueueFamily()
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

void ImGuiDlg::createDevice()
{
	int device_extension_count = 1;
	const char* device_extensions[] = { "VK_KHR_swapchain" };
	const float queue_priority[] = { 1.0f };
	VkDeviceQueueCreateInfo queue_info[1] = {};
	queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[0].queueFamilyIndex = ImGuiDlg::queueFamily();
	queue_info[0].queueCount = 1;
	queue_info[0].pQueuePriorities = queue_priority;
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
	create_info.pQueueCreateInfos = queue_info;
	create_info.enabledExtensionCount = device_extension_count;
	create_info.ppEnabledExtensionNames = device_extensions;
	auto err = vkCreateDevice(ImGuiDlg::physicalDevice(), &create_info, nullptr, &_device);
	vkGetDeviceQueue(_device, ImGuiDlg::queueFamily(), 0, &_queue);
}

void ImGuiDlg::createDescriptorPool()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	auto err = vkCreateDescriptorPool(_device, &pool_info, _allocator, &_descriptorPool);

}

void ImGuiDlg::test()
{
}

void ImGuiDlg::initImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForVulkan(_window);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = _instance;
	init_info.PhysicalDevice = _physicalDevice;
	init_info.Device = _device;
	init_info.QueueFamily = _queueFamily;
	init_info.Queue = _queue;
	init_info.PipelineCache = _pipelineCache;
	init_info.DescriptorPool = _descriptorPool;
	init_info.Allocator = _allocator;
	init_info.MinImageCount = MinImageCount;
	init_info.ImageCount = _vulkanWindow->ImageCount;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, _vulkanWindow->RenderPass);

	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Use any command queue
	VkCommandPool command_pool = _vulkanWindow->Frames[_vulkanWindow->FrameIndex].CommandPool;
	VkCommandBuffer command_buffer = _vulkanWindow->Frames[_vulkanWindow->FrameIndex].CommandBuffer;

	auto err = vkResetCommandPool(_device, command_pool, 0);
	check_vk_result(err);
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	err = vkBeginCommandBuffer(command_buffer, &begin_info);
	check_vk_result(err);

	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &command_buffer;
	err = vkEndCommandBuffer(command_buffer);
	check_vk_result(err);
	err = vkQueueSubmit(_queue, 1, &end_info, VK_NULL_HANDLE);
	check_vk_result(err);

	err = vkDeviceWaitIdle(_device);
	check_vk_result(err);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiDlg::FrameRender(ImDrawData* draw_data)
{
	auto wd = _vulkanWindow.get();

	VkResult err;
	VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
	VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
	err = vkAcquireNextImageKHR(_device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		_swapChainRebuild = true;
		return;
	}
	check_vk_result(err);

	ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
	{
		err = vkWaitForFences(_device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
		check_vk_result(err);

		err = vkResetFences(_device, 1, &fd->Fence);
		check_vk_result(err);
	}
	{
		err = vkResetCommandPool(_device, fd->CommandPool, 0);
		check_vk_result(err);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
		check_vk_result(err);
	}
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = wd->RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = wd->Width;
		info.renderArea.extent.height = wd->Height;
		info.clearValueCount = 1;
		info.pClearValues = &wd->ClearValue;
		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

	// Submit command buffer
	vkCmdEndRenderPass(fd->CommandBuffer);
	{
		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &image_acquired_semaphore;
		info.pWaitDstStageMask = &wait_stage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &fd->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &render_complete_semaphore;

		err = vkEndCommandBuffer(fd->CommandBuffer);
		check_vk_result(err);
		err = vkQueueSubmit(_queue, 1, &info, fd->Fence);
		check_vk_result(err);
	}
}

void ImGuiDlg::FramePresent()
{
	if (_swapChainRebuild)
		return;
	VkSemaphore render_complete_semaphore = _vulkanWindow->FrameSemaphores
		[_vulkanWindow->SemaphoreIndex].RenderCompleteSemaphore;
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &render_complete_semaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &_vulkanWindow->Swapchain;
	info.pImageIndices = &_vulkanWindow->FrameIndex;
	VkResult err = vkQueuePresentKHR(_queue, &info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		_swapChainRebuild = true;
		return;
	}
	check_vk_result(err);
	_vulkanWindow->SemaphoreIndex = (_vulkanWindow->SemaphoreIndex + 1) % _vulkanWindow->ImageCount;
}
