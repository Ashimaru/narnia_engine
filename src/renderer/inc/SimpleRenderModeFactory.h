#pragma once
#include "AbstractRenderModeFactory.h"
#include "SimpleRenderMode.h"

class SimpleRenderModeFactory : public AbstractRenderModeFactory
{
public:
	SimpleRenderModeFactory(GPUPtr &gpu, const ScenePtr &scene);
	~SimpleRenderModeFactory() override = default;

	RenderModePtr createRenderMode(vk::Format swapchainFormat, vk::Extent2D extent, const std::vector<vk::PipelineShaderStageCreateInfo> &shaders) override;

protected:
	void recordCommandBuffers();
	bool createRenderPass(vk::Format swapchainFormat);
	void createPipelineLayout();
	bool createPipeline(const std::vector<vk::PipelineShaderStageCreateInfo> &shaders, const vk::Viewport &viewport, const vk::Rect2D &scissors);


	bool createSwapchain(vk::Extent2D extent);
	void createCommandPool();
	SimpleRenderModePtr m_renderModeInstance;
	GPUPtr m_gpu;
	ScenePtr m_scene;
};

