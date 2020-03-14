#pragma once
#include <memory>
#include <vector>
#include "vulkan\vulkan.hpp"

#include "RenderableObject.h"
#include "ResourceManagerAPI.h"

class Scene
{
public:
	Scene();
	~Scene();

	std::vector<RenderableObjectPtr> renderableObjects;
};

using ScenePtr = std::shared_ptr<Scene>;

