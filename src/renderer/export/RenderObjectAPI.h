#pragma once
#include <memory>
#include <glm/glm.hpp>

class RenderableObjectAPI
{
public:
	virtual bool isActive() = 0;
	virtual void updatePosition(glm::vec3 newPostion) = 0;

	virtual ~RenderableObjectAPI() = default;
};

using RenderableObjectAPIPtr = std::shared_ptr<RenderableObjectAPI>;