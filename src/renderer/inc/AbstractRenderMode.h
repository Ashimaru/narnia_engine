#pragma once
#include "vulkan\vulkan.hpp"
#include "GPU.h"

class AbstractRenderMode
{
public:
	virtual ~AbstractRenderMode() = default;

	virtual void cleanUp() = 0;
	virtual vk::CommandBuffer* getCommandBuffer(const size_t index) = 0;

protected:
	vk::Pipeline m_pipeline;
	vk::RenderPass m_renderPass;
	vk::PipelineLayout m_pipelineLayout;

	vk::CommandPool m_commandPool;

	std::vector<vk::CommandBuffer> m_commandBuffers;
	std::vector<vk::Framebuffer> m_swapchainFramebuffers;

	friend class AbstractRenderModeFactory;
};


using RenderModePtr = std::unique_ptr<AbstractRenderMode>;
