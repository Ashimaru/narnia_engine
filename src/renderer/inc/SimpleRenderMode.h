#pragma once
#include "vulkan\vulkan.hpp"

struct SimpleRenderMode
{
	vk::Pipeline pipeline;
	vk::RenderPass renderPass;
	vk::PipelineLayout pipelineLayout;

	vk::CommandPool commandPool;

	std::vector<vk::CommandBuffer> commandBuffers;
	std::vector<vk::Framebuffer> swapchainFramebuffers;
};