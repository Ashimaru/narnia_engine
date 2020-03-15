#pragma once
#include <memory>
#include <vector>

#include "RenderableObject.h"
#include "ResourceManagerAPI.h"

struct Scene
{
	std::vector<RenderableObjectPtr> renderableObjects;
};

using ScenePtr = std::shared_ptr<Scene>;

