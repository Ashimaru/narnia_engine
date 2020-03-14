#include "GPUFactory.h"
#include "LoggerAPI.h"

#include <set>
#include <vector>
#include <memory>

namespace {
constexpr float QUEUE_PRIORITY = 1.0f;
constexpr int QUEUE_COUNT = 1;
}

const std::vector<const char *> deviceRequiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

GPUPtr GPUFactory::createGPU(const vk::Instance & vulkanInstance, const vk::SurfaceKHR &surface, const std::vector<const char *>& enabledValidationLayers)
{
	std::shared_ptr<GPU> gpu(new GPU());

	auto possibleDevices = getPossibleDevices(vulkanInstance);
	if (possibleDevices.empty())
	{
		LoggerAPI::getLogger()->logCritical("Could not find suitable device");
		return nullptr;
	}
	gpu->physicalDevice = chooseDevice(possibleDevices);

	gpu->queueIndexes = getPhysicalDeviceQueueProperties(surface, gpu->physicalDevice);
	if (gpu->queueIndexes == nullptr)
	{
		LoggerAPI::getLogger()->logCritical("Could not find suitable queues");
		return nullptr;
	}

	createDevice(gpu, enabledValidationLayers);
	acquireQueueHandles(gpu);

	createTransferCommandPool(gpu);

	getSwapChainSupportDetails(gpu, surface);
	if (gpu->m_swapchainDetails.surfaceFormats.empty() || gpu->m_swapchainDetails.presentModes.empty())
	{
		LoggerAPI::getLogger()->logCritical("Failed to acquire swapchain datails");
		return false;
	}

	createSwapchain(gpu, surface);
	getSwapchainImages(gpu);
	createImageViews(gpu);

	return gpu;
}


std::vector<vk::PhysicalDevice> GPUFactory::getPossibleDevices(const vk::Instance &vulkanInstance)
{
	const std::vector<vk::PhysicalDevice> devices = vulkanInstance.enumeratePhysicalDevices();

	LoggerAPI::getLogger()->logInfo("Found " + std::to_string(devices.size()) + " suitable device(s).");

	return devices;

}

vk::PhysicalDevice GPUFactory::chooseDevice(const std::vector<vk::PhysicalDevice>& devices)
{
	if (devices.size() == 0)
		return nullptr;

	for (const auto& device : devices)
	{
		auto requiredExtensions = std::vector<std::string>(std::begin(deviceRequiredExtensions), std::end(deviceRequiredExtensions));

		const auto supportedExtensions = device.enumerateDeviceExtensionProperties();

		for (const vk::ExtensionProperties &extension : supportedExtensions)
		{
			auto it = std::find(std::begin(requiredExtensions), std::end(requiredExtensions), extension.extensionName);
			if (it != end(requiredExtensions))
			{
				requiredExtensions.erase(it);
				if (requiredExtensions.size() == 0)
					return device;
			}
		}
	}

	return nullptr;
}

QueueFamilies* GPUFactory::getPhysicalDeviceQueueProperties(const vk::SurfaceKHR &surface, const vk::PhysicalDevice &physicalDevice)
{
	auto queueIndexes = new QueueFamilies();
	auto queueFamilies = physicalDevice.getQueueFamilyProperties();

	for (size_t i = 0; i < queueFamilies.size(); ++i)
	{
		auto queueFamily = queueFamilies[i];
		if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
			queueIndexes->presentationFamilyIndex = static_cast<int>(i);

		if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
			queueIndexes->graphicsFamilyIndex = static_cast<int>(i);

		if (queueFamilies[i].queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) && !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
			queueIndexes->transferFamilyIndex = static_cast<int>(i);
	}

	if (queueIndexes->graphicsFamilyIndex < 0 || queueIndexes->presentationFamilyIndex < 0 || queueIndexes->transferFamilyIndex < 0)
		return nullptr;
	return queueIndexes;
}

void GPUFactory::createDevice(GPUPtr &gpu, const std::vector<const char *>& enabledValidationLayers)
{
	auto reqPhysDevFeat = gpu->physicalDevice.getFeatures();
	auto deviceQueueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();
	
	std::set<int> uniqueQueueFamilies = { gpu->queueIndexes->graphicsFamilyIndex, gpu->queueIndexes->presentationFamilyIndex, gpu->queueIndexes->transferFamilyIndex };

	for (int queueFamily : uniqueQueueFamilies) {

		auto queueCreateInfo = vk::DeviceQueueCreateInfo{};
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &QUEUE_PRIORITY;

		deviceQueueCreateInfos.push_back(queueCreateInfo);
	}

	auto deviceCreateInfo = vk::DeviceCreateInfo();
	deviceCreateInfo.setPQueueCreateInfos(deviceQueueCreateInfos.data());
	deviceCreateInfo.setQueueCreateInfoCount(static_cast<uint32_t>(deviceQueueCreateInfos.size()));
	deviceCreateInfo.setPEnabledFeatures(&reqPhysDevFeat);
	deviceCreateInfo.setEnabledExtensionCount(static_cast<uint32_t>(deviceRequiredExtensions.size()));
	deviceCreateInfo.setPpEnabledExtensionNames(deviceRequiredExtensions.data());
	deviceCreateInfo.setEnabledLayerCount(static_cast<uint32_t>(enabledValidationLayers.size()));
	deviceCreateInfo.setPpEnabledLayerNames(enabledValidationLayers.data());

	gpu->physicalDevice.createDevice(&deviceCreateInfo, nullptr, &gpu->m_device);
}

void GPUFactory::createTransferCommandPool(GPUPtr & gpu)
{
	const auto tranferCommandPoolCreateInfo = vk::CommandPoolCreateInfo{
		vk::CommandPoolCreateFlags(),
		(uint32_t)gpu->queueIndexes->transferFamilyIndex
	};

	gpu->m_transferCommandPool = gpu->m_device.createCommandPool(tranferCommandPoolCreateInfo);
}

void GPUFactory::acquireQueueHandles(GPUPtr & gpu)
{
	gpu->graphicsQueue = gpu->m_device.getQueue(gpu->queueIndexes->graphicsFamilyIndex, 0);
	gpu->presentationQueue = gpu->m_device.getQueue(gpu->queueIndexes->presentationFamilyIndex, 0);
	gpu->transferQueue = gpu->m_device.getQueue(gpu->queueIndexes->transferFamilyIndex, 0);
}

void GPUFactory::getSwapChainSupportDetails(GPUPtr &gpu, const vk::SurfaceKHR & surface)
{
	gpu->m_swapchainDetails.surfaceCapabilites = gpu->physicalDevice.getSurfaceCapabilitiesKHR(surface);
	gpu->m_swapchainDetails.surfaceFormats = gpu->physicalDevice.getSurfaceFormatsKHR(surface);
 	gpu->m_swapchainDetails.presentModes = gpu->physicalDevice.getSurfacePresentModesKHR(surface);
}

void GPUFactory::createSwapchain(GPUPtr &gpu, const vk::SurfaceKHR &surface)
{
	const auto capabilities = gpu->m_swapchainDetails.surfaceCapabilites;

	auto surfaceFormat = chooseSurfaceFormat(gpu);
	auto presentMode = choosePresentMode(gpu);
	chooseSwapchainExtent(gpu);

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	std::string swapChainInfo = "Creating swapchain with " + std::to_string(imageCount) + " images, surface fromat is: " +
		to_string(surfaceFormat.format) + ", color space is: " + to_string(surfaceFormat.colorSpace) +
		". Presenting in: " + to_string(presentMode) + " mode.";

	LoggerAPI::getLogger()->logInfo(swapChainInfo);

	auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR();

	swapchainCreateInfo.setSurface(surface);
	swapchainCreateInfo.setMinImageCount(imageCount);
	swapchainCreateInfo.setImageColorSpace(surfaceFormat.colorSpace);
	swapchainCreateInfo.setImageFormat(surfaceFormat.format);
	swapchainCreateInfo.setImageExtent(gpu->m_swapchainExtent);
	swapchainCreateInfo.setImageArrayLayers(1);
	swapchainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment);
	swapchainCreateInfo.setPreTransform(gpu->m_swapchainDetails.surfaceCapabilites.currentTransform);
	swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	swapchainCreateInfo.setPresentMode(presentMode);
	swapchainCreateInfo.setOldSwapchain(nullptr);

	uint32_t queueFamilyIndices[] = { (uint32_t)gpu->queueIndexes->graphicsFamilyIndex, (uint32_t)gpu->queueIndexes->presentationFamilyIndex, (uint32_t)gpu->queueIndexes->transferFamilyIndex };

	if (gpu->queueIndexes->graphicsFamilyIndex != gpu->queueIndexes->presentationFamilyIndex)
	{
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchainCreateInfo.queueFamilyIndexCount = 3;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
		swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
		swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
	}


	gpu->swapchain = gpu->m_device.createSwapchainKHR(swapchainCreateInfo);
	gpu->m_swapchainFormat = surfaceFormat.format;
}

vk::SurfaceFormatKHR GPUFactory::chooseSurfaceFormat(const GPUPtr &gpu)
{
	if (gpu->m_swapchainDetails.surfaceFormats.size() == 1 && gpu->m_swapchainDetails.surfaceFormats[0].format == vk::Format::eA8B8G8R8UnormPack32)
	{
		return { vk::Format::eA8B8G8R8UnormPack32, vk::ColorSpaceKHR::eSrgbNonlinear };
	}

	const auto it = std::find_if(std::cbegin(gpu->m_swapchainDetails.surfaceFormats), std::cend(gpu->m_swapchainDetails.surfaceFormats), [](const auto& format)
	{
		return format.format == vk::Format::eA8B8G8R8UnormPack32 && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
	});

	if(it == std::cend(gpu->m_swapchainDetails.surfaceFormats))
	{
		LoggerAPI::getLogger()->logWarning("Preferred format not found returning default");
		return gpu->m_swapchainDetails.surfaceFormats[0];
	}

	return *it;
}

vk::PresentModeKHR GPUFactory::choosePresentMode(const GPUPtr &gpu)
{
	auto result = vk::PresentModeKHR::eFifo;

	for (const auto &presentMode : gpu->m_swapchainDetails.presentModes)
	{
		if (presentMode == vk::PresentModeKHR::eMailbox)
			return presentMode;
		else if (presentMode == vk::PresentModeKHR::eImmediate)
			result = presentMode;
	}

	return result;
}

void GPUFactory::chooseSwapchainExtent(GPUPtr &gpu)
{
	const auto capabilities = gpu->m_swapchainDetails.surfaceCapabilites;
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		gpu->m_swapchainExtent = capabilities.currentExtent;

	auto result = vk::Extent2D();

	result.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, result.width));
	result.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, result.height));

	gpu->m_swapchainExtent = result;
}

void GPUFactory::getSwapchainImages(GPUPtr &gpu)
{
	gpu->m_swapchainImages = gpu->m_device.getSwapchainImagesKHR(gpu->swapchain);
}

void GPUFactory::createImageViews(GPUPtr &gpu)
{
	auto resourceSubrange = vk::ImageSubresourceRange();
	resourceSubrange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	resourceSubrange.setLevelCount(1);
	resourceSubrange.setLayerCount(1);

	gpu->m_swapchainImageViews.resize(gpu->m_swapchainImages.size());

	for (size_t i = 0; i < gpu->m_swapchainImages.size(); ++i)
	{
		auto createInfo = vk::ImageViewCreateInfo();
		createInfo.setImage(gpu->m_swapchainImages[i]);
		createInfo.setFormat(gpu->m_swapchainFormat);
		createInfo.setSubresourceRange(resourceSubrange);
		createInfo.setViewType(vk::ImageViewType::e2D);

		gpu->m_swapchainImageViews[i] = gpu->m_device.createImageView(createInfo);
	}

}
