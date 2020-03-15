#pragma once
#include "ResourceManagerAPI.h"
#include "ModelResource.h"

class ResourceManager : public ResourceManagerAPI
{
public:
	ResourceManager() = default;
	~ResourceManager() override = default;

	void LoadResources() override;
	const ShaderResource& getShader(const std::string &shaderName) const override;
	ModelData getModel(const std::string &modelName) override;

	void cleanUp() override;


private:
	
	std::vector<ShaderResource> m_shaderModules;
	std::vector<ModelResource>  m_models;

	void loadShaders();
	void loadModels();
	void createShader(const std::string &metaData);
	void unloadShaders();
	void unloadModels();

};
using ResourceManagerPtr = std::shared_ptr<ResourceManager>;
