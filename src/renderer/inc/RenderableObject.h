#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>

#include "RenderObjectAPI.h"
#include "glm/glm.hpp"


class RenderableObject : public RenderableObjectAPI
{
public:
	explicit RenderableObject(std::string name, glm::vec3 position = {});

	bool isActive() override;
	void updatePosition(glm::vec3 newPosition) override;

	vk::Buffer sharedBuffer;
	vk::DeviceMemory sharedBufferMemory;
	uint32_t indexCount;
	vk::DeviceSize vertexOffset;

	glm::vec3 m_position;

	~RenderableObject() override = default;

private:
	const std::string m_name;
	bool m_active;
};

using RenderableObjectPtr = std::shared_ptr<RenderableObject>;