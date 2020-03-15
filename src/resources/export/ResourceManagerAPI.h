#pragma once
#include <memory>
#include "ShaderResource.h"
#include "ResourceDefs.h"

class ResourceManagerAPI;
using ResourceManagerAPIPtr = std::shared_ptr<ResourceManagerAPI>;
class ResourceManagerAPI
{
public:
	virtual ~ResourceManagerAPI() = default;

	virtual void LoadResources() = 0;
	virtual const ShaderResource& getShader(const std::string &shaderName) const = 0;
	virtual ModelData getModel(const std::string &modelName) = 0;
	virtual void cleanUp() = 0;
	
	static ResourceManagerAPIPtr createInstance();
};
