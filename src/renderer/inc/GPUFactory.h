#pragma once
#include "GPU.h"

class GPUFactory
{
public:
	static GPUPtr createGPU(const vk::Instance & vulkanInstance, const vk::SurfaceKHR &surface, const std::vector<const char *>& enabledValidationLayers);

private:
	static std::vector<vk::PhysicalDevice> getPossibleDevices(const vk::Instance & vulkanInstance);
	static vk::PhysicalDevice GPUFactory::chooseDevice(const std::vector<vk::PhysicalDevice>& devices);
	static QueueFamilies* GPUFactory::getPhysicalDeviceQueueProperties(const vk::SurfaceKHR &surface, const vk::PhysicalDevice &physicalDevice);
	static void createDevice(GPUPtr &gpu, const std::vector<const char *>& enabledValidationLayers);
	static void createTransferCommandPool(GPUPtr &gpu);
	static void acquireQueueHandles(GPUPtr &gpu);
	static void getSwapChainSupportDetails(GPUPtr &gpu, const vk::SurfaceKHR &surface);
	static void createSwapchain(GPUPtr & gpu, const vk::SurfaceKHR & surface);
	static vk::SurfaceFormatKHR chooseSurfaceFormat(const GPUPtr & gpu);
	static vk::PresentModeKHR choosePresentMode(const GPUPtr & gpu);
	static void chooseSwapchainExtent(GPUPtr & gpu);
	static void getSwapchainImages(GPUPtr &gpu);
	static void createImageViews(GPUPtr & gpu);
};

