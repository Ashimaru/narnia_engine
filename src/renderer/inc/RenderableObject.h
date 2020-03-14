#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>

#include "RenderObjectAPI.h"


class RenderableObject : public RenderableObjectAPI
{
public:
	explicit RenderableObject(std::string name);

	bool isActive() override;

	vk::Buffer sharedBuffer;
	vk::DeviceMemory sharedBufferMemory;

	uint32_t indexCount;
	vk::DeviceSize vertexOffset;

	~RenderableObject() override = default;

private:
	std::string m_name;
	bool m_active;
};

using RenderableObjectPtr = std::shared_ptr<RenderableObject>;