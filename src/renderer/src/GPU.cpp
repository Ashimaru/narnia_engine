#include "GPU.h"
#include "LoggerAPI.h"
#include "RenderEngine.h"

#include <set>
#include <algorithm>

using std::string;
using std::vector;
using std::set;

using Devices = vector<vk::PhysicalDevice>;
const auto stagingBufferUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc;
const auto stagingBufferMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

const auto targetBufferUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
const auto targetBufferMemoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

const auto waitForFenceTimer = std::numeric_limits<uint64_t>::max();
const auto imageAquirementTimer = 2;

GPU::~GPU()
{
	delete queueIndexes;
}

void GPU::waitForFence(vk::Fence * fence) const
{
	m_device.waitForFences(1, fence, true, waitForFenceTimer);
}

void GPU::resetFence(const vk::Fence * fence) const
{
	m_device.resetFences(1, fence);
}

void GPU::acquireNextImage(const vk::Semaphore &semaphore) const
{
	m_device.acquireNextImageKHR(swapchain, imageAquirementTimer, semaphore, nullptr);
}

void GPU::submitToGraphicsQueue(const vk::SubmitInfo *submitInfo, vk::Fence fence) const
{
	graphicsQueue.submit(1, submitInfo, fence);
}

void GPU::submitToPresentationQueue(const vk::PresentInfoKHR &presentInfo) const
{
	presentationQueue.presentKHR(presentInfo);
}

uint32_t GPU::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
	auto memProperties = vk::PhysicalDeviceMemoryProperties();
	physicalDevice.getMemoryProperties(&memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;

	LoggerAPI::getLogger()->logCritical("Could not find requested memory type");
	return uint32_t{ 0 };
}

void GPU::createCommandBuffers(const vk::CommandBufferAllocateInfo & allocateInfo, vk::CommandBuffer * buffer) const
{
	m_device.allocateCommandBuffers(&allocateInfo, buffer);
}

void GPU::createSemaphore(const vk::SemaphoreCreateInfo &info, vk::Semaphore &semaphore) const
{
	if (vk::Result::eSuccess != m_device.createSemaphore(&info, nullptr, &semaphore))
		assert(false);
}

void GPU::deleteSemaphore(const vk::Semaphore & semaphore) const
{
	m_device.destroySemaphore(semaphore);
}

void GPU::createFence(const vk::FenceCreateInfo &fenceCreateInfo, vk::Fence &fence) const
{
	m_device.createFence(&fenceCreateInfo, nullptr, &fence);
}

void GPU::deleteFence(const vk::Fence & fence) const
{
	m_device.destroyFence(fence);
}

void GPU::deleteCommandBuffer(vk::CommandPool commandPool, vector<vk::CommandBuffer> &commandBuffers) const
{
	m_device.freeCommandBuffers(commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}

void GPU::waitForRender() const
{
	m_device.waitIdle();
}

void GPU::cleanUp()
{
	m_device.destroyCommandPool(m_transferCommandPool);
	for (auto &imageView : m_swapchainImageViews)
		m_device.destroyImageView(imageView);
	m_device.destroySwapchainKHR(swapchain);
	m_device.destroy();
}

vk::Extent2D GPU::getPresentationExtent() const
{
	return m_swapchainExtent;
}

vk::Format GPU::getSwapchanFormat() const
{
	return m_swapchainFormat;
}

size_t GPU::getSwapchainImagesCount() const
{
	return m_swapchainImageViews.size();
}

void GPU::loadROToMemory(const ModelResourcePtr & model, RenderableObjectPtr &renderObject) const
{
	createSharedBuffer(model->verticies, model->indicies, renderObject->sharedBuffer, renderObject->sharedBufferMemory);
}

void GPU::unloadROFromMemory(const RenderableObjectPtr & renderObject) const
{
	m_device.destroyBuffer(renderObject->sharedBuffer);
	m_device.freeMemory(renderObject->sharedBufferMemory);
}

void GPU::createRenderPass(vk::RenderPassCreateInfo &createInfo, vk::RenderPass &renderPass) const
{
	m_device.createRenderPass(&createInfo, nullptr, &renderPass);
}

void GPU::deleteRenderPass(const vk::RenderPass & renderPass) const
{
	m_device.destroyRenderPass(renderPass);
}

void GPU::createPipelineLayout(vk::PipelineLayoutCreateInfo & createInfo, vk::PipelineLayout &layout) const
{
	m_device.createPipelineLayout(&createInfo, nullptr, &layout);
}

void GPU::deletePipelineLayout(const vk::PipelineLayout & pipelineLayout) const
{
	m_device.destroyPipelineLayout(pipelineLayout);
}

void GPU::createPipeline(const vk::GraphicsPipelineCreateInfo & createInfo, vk::Pipeline &pipeline) const
{
	pipeline = m_device.createGraphicsPipeline(nullptr, createInfo, nullptr);
}

void GPU::deletePipeline(const vk::Pipeline & pipeline) const
{
	m_device.destroyPipeline(pipeline);
}

void GPU::createFramebuffer(vk::FramebufferCreateInfo & createInfo, int index, vk::Framebuffer &framebuffer) const
{
	vk::ImageView attachments[] = { m_swapchainImageViews[index] };

	createInfo.setPAttachments(attachments);

	m_device.createFramebuffer(&createInfo, nullptr, &framebuffer);
}

void GPU::deleteFramebuffer(const vk::Framebuffer & framebuffer) const
{
	m_device.destroyFramebuffer(framebuffer);
}

void GPU::createShaderModule(const vk::ShaderModuleCreateInfo & createInfo, vk::ShaderModule &shaderModule) const
{
	m_device.createShaderModule(&createInfo, nullptr, &shaderModule);
}

void GPU::deleteShaderModule(const vk::ShaderModule & shaderModule) const
{
	m_device.destroyShaderModule(shaderModule);
}

void GPU::createGraphicsCommandPool(vk::CommandPool &commandPool) const
{
	const auto commandPoolInfo = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(), queueIndexes->graphicsFamilyIndex);

	m_device.createCommandPool(&commandPoolInfo, nullptr, &commandPool);

}

void GPU::deleteCommandPool(const vk::CommandPool & commandPool) const
{
	m_device.destroyCommandPool(commandPool);
}

void GPU::createBuffer(const vk::DeviceSize bufferSize, const vk::BufferUsageFlags bufferUsageFlags, const vk::MemoryPropertyFlags /*memoryPropertyFlags*/, vk::Buffer &buffer, vk::DeviceMemory &deviceMemory) const
{
	auto bufferCreateInfo = vk::BufferCreateInfo();
	bufferCreateInfo.setSize(bufferSize);
	bufferCreateInfo.setUsage(bufferUsageFlags);
	bufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);

	buffer = m_device.createBuffer(bufferCreateInfo);

	allocateMemoryForBuffer(buffer, deviceMemory);
	m_device.bindBufferMemory(buffer, deviceMemory, 0);
}

void GPU::copyBuffer(const vk::Buffer & sourceBuffer, const vk::Buffer & destBuffer, const vk::DeviceSize bufferSize) const
{
	auto commandBufferAlloccateInfo = vk::CommandBufferAllocateInfo{};
	commandBufferAlloccateInfo.setCommandBufferCount(1);
	commandBufferAlloccateInfo.setCommandPool(m_transferCommandPool);
	commandBufferAlloccateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

	vk::CommandBuffer transferCommandBuffer;
	m_device.allocateCommandBuffers(&commandBufferAlloccateInfo, &transferCommandBuffer);

	auto commandBufferBegin = vk::CommandBufferBeginInfo{};
	commandBufferBegin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	transferCommandBuffer.begin(&commandBufferBegin);

	auto bufferCopy = vk::BufferCopy{};
	bufferCopy.setDstOffset(0);
	bufferCopy.setSrcOffset(0);
	bufferCopy.setSize(bufferSize);

	transferCommandBuffer.copyBuffer(sourceBuffer, destBuffer, 1, &bufferCopy);
	transferCommandBuffer.end();

	auto submitInfo = vk::SubmitInfo{};
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&transferCommandBuffer);

	transferQueue.submit(1, &submitInfo, nullptr);
	transferQueue.waitIdle();

	m_device.freeCommandBuffers(m_transferCommandPool, 1, &transferCommandBuffer);
}

void GPU::allocateMemoryForBuffer(const vk::Buffer & buffer, vk::DeviceMemory & deviceMemory) const
{
	vk::MemoryRequirements memReq;
	m_device.getBufferMemoryRequirements(buffer, &memReq);

	vk::MemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.setAllocationSize(memReq.size);
	memoryAllocateInfo.setMemoryTypeIndex(findMemoryType(memReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

	if (vk::Result::eSuccess != m_device.allocateMemory(&memoryAllocateInfo, nullptr, &deviceMemory))
		LoggerAPI::getLogger()->logError("Failed to allocate memory for buffer");


}

void GPU::createSharedBuffer(const std::vector<Vertex>& verticies, const std::vector<uint32_t>& indicies, vk::Buffer & buffer, vk::DeviceMemory & deviceMemory) const
{
	vk::DeviceSize verticiesSize = sizeof(Vertex) * verticies.size();
	vk::DeviceSize indiciesSize = sizeof(uint32_t) * indicies.size();

	vk::DeviceSize bufferSize = verticiesSize + indiciesSize;

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, stagingBufferUsageFlags, stagingBufferMemoryProperties, stagingBuffer, stagingBufferMemory);

	void *data;
	data = m_device.mapMemory(stagingBufferMemory, 0, verticiesSize, vk::MemoryMapFlags());
	memcpy(data, verticies.data(), verticiesSize);
	m_device.unmapMemory(stagingBufferMemory);

	data = m_device.mapMemory(stagingBufferMemory, verticiesSize, indiciesSize, vk::MemoryMapFlags());
	memcpy(data, indicies.data(), indiciesSize);
	m_device.unmapMemory(stagingBufferMemory);

	createBuffer(bufferSize, targetBufferUsageFlags, targetBufferMemoryProperties, buffer, deviceMemory);

	copyBuffer(stagingBuffer, buffer, bufferSize);

	m_device.destroyBuffer(stagingBuffer);
	m_device.freeMemory(stagingBufferMemory);
}
