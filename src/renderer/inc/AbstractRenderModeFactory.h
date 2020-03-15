#pragma once
#include "SimpleRenderMode.h"
#include "GPU.h"
#include "Scene.h"

class AbstractRenderModeFactory
{
public:
	virtual ~AbstractRenderModeFactory() = default;

	virtual SimpleRenderMode createRenderMode(vk::Format swapchainFormat, vk::Extent2D extent, const std::vector<vk::PipelineShaderStageCreateInfo> &shaders) = 0;
};

using RenderModeFactoryPtr = std::shared_ptr<AbstractRenderModeFactory>;

