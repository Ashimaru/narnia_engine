#pragma once
#include "RenderableObject.h"
#include "ModelResource.h"
#include "SimpleRenderMode.h"


struct QueueFamilies {
	int graphicsFamilyIndex = -1;
	int presentationFamilyIndex = -1;
	int transferFamilyIndex = -1;
};

class GPU
{
public:
	~GPU();

	void waitForRender() const;
	void cleanUp();
	
	void waitForFence(vk::Fence *fence) const;
	void resetFence(const vk::Fence *fence) const;
	void acquireNextImage(const vk::Semaphore &semaphore) const;
	void submitToGraphicsQueue(const vk::SubmitInfo *submitInfo, vk::Fence fence) const;
	void submitToPresentationQueue(const vk::PresentInfoKHR &presentInfo) const;

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

	void deleteRenderMode(SimpleRenderMode && mode) const;
	
	void loadROToMemory(const ModelResourcePtr &model, RenderableObjectPtr &renderObject) const;
	void unloadROFromMemory(const RenderableObjectPtr &renderObject) const;

	void createRenderPass(vk::RenderPassCreateInfo &createInfo, vk::RenderPass &renderPass) const;
	void deleteRenderPass(const vk::RenderPass &renderPass) const;

	void createPipelineLayout(vk::PipelineLayoutCreateInfo &createInfo, vk::PipelineLayout &layout) const;
	void deletePipelineLayout(const vk::PipelineLayout &pipelineLayout) const;

	void createPipeline(const vk::GraphicsPipelineCreateInfo &createInfo, vk::Pipeline &pipeline) const;
	void deletePipeline(const vk::Pipeline &pipeline) const;

	void createFramebuffer(vk::FramebufferCreateInfo &createInfo, int index, vk::Framebuffer &framebuffer) const;
	void deleteFramebuffer(const vk::Framebuffer &framebuffer) const;

	void createShaderModule(const vk::ShaderModuleCreateInfo &createInfo, vk::ShaderModule &shaderModule) const;
	void deleteShaderModule(const vk::ShaderModule &shaderModule) const;

	void createGraphicsCommandPool(vk::CommandPool &commandPool) const;
	void deleteCommandPool(const vk::CommandPool &commandPool) const;

	void createCommandBuffers(const vk::CommandBufferAllocateInfo &allocateInfo, vk::CommandBuffer *buffer) const;
	void deleteCommandBuffer(const vk::CommandPool commandPool, std::vector<vk::CommandBuffer> &commandBuffers) const;

	void createSemaphore(const vk::SemaphoreCreateInfo &info, vk::Semaphore &semaphore) const;
	void deleteSemaphore(const vk::Semaphore &semaphore) const;

	void createFence(const vk::FenceCreateInfo &fenceCreateInfo, vk::Fence& fence) const;
	void deleteFence(const vk::Fence &fence) const;

	vk::Extent2D getPresentationExtent() const;
	vk::Format getSwapchanFormat() const;

	size_t getSwapchainImagesCount() const;

	vk::SwapchainKHR swapchain = nullptr;
	struct QueueFamilies* queueIndexes;

private:
	GPU() = default;

	void createBuffer(const vk::DeviceSize bufferSize, const vk::BufferUsageFlags bufferUsageFlags,
		const vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer &buffer, vk::DeviceMemory &deviceMemory) const;
	void copyBuffer(const vk::Buffer &sourceBuffer, const vk::Buffer &destBuffer, const vk::DeviceSize bufferSize) const;
	void allocateMemoryForBuffer(const vk::Buffer &buffer, vk::DeviceMemory &deviceMemory) const;

	void createSharedBuffer(const std::vector<Vertex>& verticies, const std::vector<uint32_t>& indicies, vk::Buffer &buffer, vk::DeviceMemory &deviceMemory) const;

	vk::Device m_device;
	vk::PhysicalDevice physicalDevice;
	std::vector<vk::Image> m_swapchainImages;
	std::vector<vk::ImageView> m_swapchainImageViews;
	vk::CommandPool m_transferCommandPool;

	vk::Format m_swapchainFormat = vk::Format::eUndefined;
	vk::Extent2D m_swapchainExtent = vk::Extent2D(0, 0);

	vk::Queue graphicsQueue = nullptr;
	vk::Queue presentationQueue = nullptr;
	vk::Queue transferQueue = nullptr;

	struct SwapchainSupportDetails {
		vk::SurfaceCapabilitiesKHR surfaceCapabilites;
		std::vector<vk::SurfaceFormatKHR> surfaceFormats = std::vector<vk::SurfaceFormatKHR>(0);
		std::vector<vk::PresentModeKHR> presentModes = std::vector<vk::PresentModeKHR>(0);
	}m_swapchainDetails;

	friend class GPUFactory;
};
using GPUPtr = std::shared_ptr<GPU>;