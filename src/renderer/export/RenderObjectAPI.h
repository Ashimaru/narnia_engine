#pragma once
#include <memory>


class RenderableObjectAPI
{
public:
	virtual bool isActive() = 0;

	virtual ~RenderableObjectAPI() = default;
};

using RenderableObjectAPIPtr = std::shared_ptr<RenderableObjectAPI>;