#include "SimpleRenderMode.h"


SimpleRenderMode::SimpleRenderMode(GPUPtr &gpu) :
	m_gpu{gpu}
{
}

void SimpleRenderMode::cleanUp()
{
	m_gpu->deleteCommandBuffer(m_commandPool, m_commandBuffers);
	m_gpu->deleteCommandPool(m_commandPool);
	m_gpu->deletePipelineLayout(m_pipelineLayout);
	m_gpu->deletePipeline(m_pipeline);

	for (auto framebuffer : m_swapchainFramebuffers)
		m_gpu->deleteFramebuffer(framebuffer);
	m_gpu->deleteRenderPass(m_renderPass);
}

vk::CommandBuffer * SimpleRenderMode::getCommandBuffer(const size_t index)
{
	return &(m_commandBuffers[index]);
}


