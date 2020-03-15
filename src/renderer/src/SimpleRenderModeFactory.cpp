#include "SimpleRenderModeFactory.h"
#include "LoggerAPI.h"
#include "Vertex.h"

#include <array>
#include <cassert>

using std::array;

SimpleRenderModeFactory::SimpleRenderModeFactory(GPUPtr &gpu, const ScenePtr &scene) :
	m_gpu(gpu),
	m_scene(scene)
{
} 

SimpleRenderMode SimpleRenderModeFactory::createRenderMode(vk::Format swapchainFormat, vk::Extent2D extent, const std::vector<vk::PipelineShaderStageCreateInfo> &shaders)
{
	assert(createRenderPass(swapchainFormat));

	createPipelineLayout();

	auto viewport = vk::Viewport();
	viewport.setWidth((float)extent.width);
	viewport.setHeight((float)extent.height);
	viewport.setX(0);
	viewport.setY(0);
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	auto scissors = vk::Rect2D();
	scissors.setOffset({ 0,0 });
	scissors.setExtent(extent);

	assert(createPipeline(shaders, viewport, scissors));

	assert(createSwapchain(extent));

	createCommandPool();

	recordCommandBuffers();

	return m_result;
}

void SimpleRenderModeFactory::recordCommandBuffers()
{
	m_result.commandBuffers.resize(m_result.swapchainFramebuffers.size());

	auto commandBufferAllocInfo = vk::CommandBufferAllocateInfo();
	commandBufferAllocInfo.setCommandBufferCount(static_cast<uint32_t>(m_result.commandBuffers.size()));
	commandBufferAllocInfo.setCommandPool(m_result.commandPool);
	commandBufferAllocInfo.setLevel(vk::CommandBufferLevel::ePrimary);


	m_gpu->createCommandBuffers(commandBufferAllocInfo, m_result.commandBuffers.data());

	for (size_t i = 0; i < m_result.commandBuffers.size(); ++i)
	{
		auto beginInfo = vk::CommandBufferBeginInfo();
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

		if (vk::Result::eSuccess != m_result.commandBuffers[i].begin(&beginInfo))
		{
			LoggerAPI::getLogger()->logCritical("Could not begin record command buffer");
		}

		auto renderPassBeginInfo = vk::RenderPassBeginInfo();
		renderPassBeginInfo.setRenderPass(m_result.renderPass);
		renderPassBeginInfo.setFramebuffer(m_result.swapchainFramebuffers[i]);
		auto renderArea = vk::Rect2D({ 0,0 }, m_gpu->getPresentationExtent());
		renderPassBeginInfo.setRenderArea(renderArea);
		renderPassBeginInfo.setClearValueCount(1);
		vk::ClearValue clearValues[] = { 0.0, 0.0, 0.0, 1.0 };
		renderPassBeginInfo.setPClearValues(clearValues);

		m_result.commandBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		m_result.commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_result.pipeline);
		for (const auto &ro : m_scene->renderableObjects)
		{
			vk::DeviceSize offset[] = { 0 };

			m_result.commandBuffers[i].bindVertexBuffers(0, 1, &ro->sharedBuffer, offset);
			m_result.commandBuffers[i].bindIndexBuffer(ro->sharedBuffer, ro->vertexOffset, vk::IndexType::eUint32);
			m_result.commandBuffers[i].drawIndexed(ro->indexCount, 1, 0, 0, 0);

		}
		m_result.commandBuffers[i].endRenderPass();

		m_result.commandBuffers[i].end();
	}

}

bool SimpleRenderModeFactory::createRenderPass(vk::Format swapchainFormat)
{
	auto colorAttachment = vk::AttachmentDescription();
	colorAttachment.setFormat(swapchainFormat);
	colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
	colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
	colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
	colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	auto attatchmentRef = vk::AttachmentReference();
	attatchmentRef.setAttachment(0);
	attatchmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto subpassDesc = vk::SubpassDescription();
	subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpassDesc.setColorAttachmentCount(1);
	subpassDesc.setPColorAttachments(&attatchmentRef);

	auto subpassDependency = vk::SubpassDependency();
	subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	subpassDependency.setDstSubpass(0);
	subpassDependency.setSrcStageMask(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput));
	subpassDependency.setSrcAccessMask(vk::AccessFlags(0));
	subpassDependency.setDstStageMask(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput));
	subpassDependency.setDstAccessMask(vk::AccessFlags(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite));


	auto renderPassInfo = vk::RenderPassCreateInfo();
	renderPassInfo.setSubpassCount(1);
	renderPassInfo.setPSubpasses(&subpassDesc);
	renderPassInfo.setAttachmentCount(1);
	renderPassInfo.setPAttachments(&colorAttachment);
	renderPassInfo.setDependencyCount(1);
	renderPassInfo.setPDependencies(&subpassDependency);

	m_gpu->createRenderPass(renderPassInfo, m_result.renderPass);

	return true;
}

void SimpleRenderModeFactory::createPipelineLayout()
{
	auto layoutCreateInfo = vk::PipelineLayoutCreateInfo();
	layoutCreateInfo.setSetLayoutCount(0);
	layoutCreateInfo.setPushConstantRangeCount(0);

	m_gpu->createPipelineLayout(layoutCreateInfo, m_result.pipelineLayout);
}

bool SimpleRenderModeFactory::createPipeline(const std::vector<vk::PipelineShaderStageCreateInfo> &shaders, const vk::Viewport &viewport, const vk::Rect2D &scissors)
{
	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo();
	pipelineCreateInfo.setStageCount(static_cast<uint32_t>(shaders.size()));
	pipelineCreateInfo.setPStages(shaders.data());

	auto bindingDescription = new vk::VertexInputBindingDescription();
	bindingDescription->setBinding(0);
	bindingDescription->setInputRate(vk::VertexInputRate::eVertex);
	bindingDescription->setStride(sizeof(Vertex));

	auto attributeDescriptions = array<vk::VertexInputAttributeDescription, 2>();
	attributeDescriptions.at(0).setBinding(0);
	attributeDescriptions.at(0).setFormat(vk::Format::eR32G32B32Sfloat);
	attributeDescriptions.at(0).setLocation(0);
	attributeDescriptions.at(0).setOffset(offsetof(Vertex, Vertex::postion));

	attributeDescriptions.at(1).setBinding(0);
	attributeDescriptions.at(1).setFormat(vk::Format::eR32G32B32A32Sfloat);
	attributeDescriptions.at(1).setLocation(1);
	attributeDescriptions.at(1).setOffset(offsetof(Vertex, Vertex::color));

	auto vertexInputState = vk::PipelineVertexInputStateCreateInfo();
	vertexInputState.setPVertexAttributeDescriptions(attributeDescriptions.data());
	vertexInputState.setPVertexBindingDescriptions(bindingDescription);
	vertexInputState.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
	vertexInputState.setVertexBindingDescriptionCount(1);

	pipelineCreateInfo.setPVertexInputState(&vertexInputState);


	auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo();
	inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);
	inputAssemblyState.setPrimitiveRestartEnable(false);

	pipelineCreateInfo.setPInputAssemblyState(&inputAssemblyState);

	auto viewportState = vk::PipelineViewportStateCreateInfo();
	viewportState.setViewportCount(1);
	viewportState.setScissorCount(1);
	viewportState.setPScissors(&scissors);
	viewportState.setPViewports(&viewport);

	pipelineCreateInfo.setPViewportState(&viewportState);


	auto rasteizerState = vk::PipelineRasterizationStateCreateInfo();
	rasteizerState.setDepthClampEnable(false);
	rasteizerState.setRasterizerDiscardEnable(false);
	rasteizerState.setPolygonMode(vk::PolygonMode::eFill);
	rasteizerState.setLineWidth(1.0f);
	rasteizerState.setCullMode(vk::CullModeFlagBits::eBack);
	rasteizerState.setFrontFace(vk::FrontFace::eClockwise);
	rasteizerState.setDepthBiasEnable(false);

	pipelineCreateInfo.setPRasterizationState(&rasteizerState);


	auto multisampleState = vk::PipelineMultisampleStateCreateInfo();
	multisampleState.setSampleShadingEnable(false);
	multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	pipelineCreateInfo.setPMultisampleState(&multisampleState);

	pipelineCreateInfo.setPDepthStencilState(nullptr);

	auto attachState = vk::PipelineColorBlendAttachmentState();
	attachState.setColorWriteMask(vk::ColorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA));
	attachState.setBlendEnable(false);
	attachState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	attachState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	attachState.setColorBlendOp(vk::BlendOp::eAdd);
	attachState.setAlphaBlendOp(vk::BlendOp::eAdd);

	auto colorBlendState = vk::PipelineColorBlendStateCreateInfo();
	colorBlendState.setLogicOpEnable(false);
	colorBlendState.setLogicOp(vk::LogicOp::eCopy);
	colorBlendState.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });
	colorBlendState.setAttachmentCount(1);
	colorBlendState.setPAttachments(&attachState);

	pipelineCreateInfo.setPColorBlendState(&colorBlendState);


	pipelineCreateInfo.setPDynamicState(nullptr); //not supported
	pipelineCreateInfo.setLayout(m_result.pipelineLayout);
	pipelineCreateInfo.setRenderPass(m_result.renderPass);
	pipelineCreateInfo.setSubpass(0);

	m_gpu->createPipeline(pipelineCreateInfo, m_result.pipeline);

	return true;
}

bool SimpleRenderModeFactory::createSwapchain(vk::Extent2D extent)
{
	m_result.swapchainFramebuffers.resize(m_gpu->getSwapchainImagesCount());

	for (int i = 0; i < m_result.swapchainFramebuffers.size(); ++i)
	{
		auto createInfo = vk::FramebufferCreateInfo();
		createInfo.setRenderPass(m_result.renderPass);
		createInfo.setAttachmentCount(1);
		createInfo.setWidth(extent.width);
		createInfo.setHeight(extent.height);
		createInfo.setLayers(1);


		m_gpu->createFramebuffer(createInfo, i, m_result.swapchainFramebuffers[i]);
	}

	return true;
}

void SimpleRenderModeFactory::createCommandPool()
{
	m_gpu->createGraphicsCommandPool(m_result.commandPool);
}

bool createCommandBuffers()
{
	return false;
}

bool createSyncObjects()
{
	return false;
}

