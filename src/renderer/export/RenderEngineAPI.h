#pragma once
#include "ResourceManagerAPI.h"
#include "RenderObjectAPI.h"

class RenderEngineAPI;
using RenderEngineAPIPtr = std::shared_ptr<RenderEngineAPI>;
class RenderEngineAPI
{
public:
	virtual ~RenderEngineAPI() = default;
	virtual int init(const ResourceManagerAPIPtr& resourceManager) = 0;
	virtual void drawScene() = 0;

	virtual bool pollForWindowClose() = 0;
	
	virtual void waitForRendererToFinish() = 0;
	virtual void cleanUp() = 0;

	virtual RenderableObjectAPIPtr createObject(std::string id, const std::string &modelName) = 0;
	virtual RenderableObjectAPIPtr createObject(std::string id, const std::string &modelName, glm::vec3 position) = 0;

	static RenderEngineAPIPtr createInstance();
};

