#include "ResourceManager.h"
#include "FileHelper.h"
#include "LoggerAPI.h"

#include <sstream>
#include <algorithm>

namespace{
const std::string SHADERS_PATH = "./Shaders/";
const std::string CONFIG_FILE = "index.lst";


ModelResource createRectangleModel()
{
	std::vector<Vertex> verticies = {
	{ { -0.9F, -0.9F, 0.0F },{ 1.0F, 0.0F, 0.0F, 1.0F } },
	{ {  0.0F,  0.0F, 0.0F },{ 0.0F, 1.0F, 0.0F, 1.0F } },
	{ { -0.9F,  0.0F, 0.0F },{ 0.0F, 0.0F, 1.0F, 1.0F } },
	{ {  0.0F, -0.9F, 0.0F },{ 1.0F, 1.0F, 1.0F, 1.0F } }
	};

	std::vector<uint32_t> indices = {
		0, 1, 2, 1, 0, 3
	};
	return ModelResource("rectangle", std::move(verticies), std::move(indices));
}

ModelResource createTriangleModel()
{
	std::vector<Vertex> verticies = {
	{ {  0.1F,  0.8F, 0.0F },{ 1.0F, 0.0F, 0.0F, 1.0F } },
	{ {  0.1F, -0.8F, 0.0F },{ 0.0F, 1.0F, 0.0F, 1.0F } },
	{ {  0.8F,  0.8F, 0.0F },{ 0.0F, 0.0F, 1.0F, 1.0F } }
	};

	std::vector<uint32_t> indices = {
		0, 1, 2
	};
	return ModelResource("triangle", std::move(verticies), std::move(indices));
}
}

ResourceManagerAPIPtr ResourceManagerAPI::createInstance()
{
	return std::make_shared<ResourceManager>();
}

void ResourceManager::LoadResources()
{
	loadShaders();
	loadModels();
}

const ShaderResource& ResourceManager::getShader(const std::string & shaderName) const
{
	auto it = find_if(std::begin(m_shaderModules), std::end(m_shaderModules), [&shaderName](const auto& shader)
	{
		return shader.name == shaderName;
	});

	assert(it != std::end(m_shaderModules));
	return *it;
}

ModelData ResourceManager::getModel(const std::string & modelName)
{
	auto it = std::find_if(begin(m_models), end(m_models), [&modelName](const ModelResource &model)
	{
		return model.name == modelName;
	});

	assert(it != std::end(m_models));

	return ModelData(it->verticies, it->indicies, it->usageCounter);
}

void ResourceManager::cleanUp()
{
	unloadShaders();
	unloadModels();
}

void ResourceManager::loadShaders()
{
	auto configFile = FileHelper::readFileLines(SHADERS_PATH + CONFIG_FILE);

	for (const auto &line : configFile)
	{
		createShader(line);
	}
}

void ResourceManager::loadModels()
{
	m_models.push_back(createRectangleModel());
	m_models.push_back(createTriangleModel());
}

void ResourceManager::createShader(const std::string & metaData)
{
	std::string name;
	std::string fileName;
	std::stringstream stream(metaData);

	stream >> name >> fileName;

	m_shaderModules.emplace_back(name, FileHelper::readFileByte(SHADERS_PATH + fileName));
}


void ResourceManager::unloadShaders()
{
	m_shaderModules.clear();
}

void ResourceManager::unloadModels()
{
	m_models.clear();
}
