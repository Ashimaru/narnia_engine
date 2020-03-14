#pragma once
#include "BasicResource.h"
#include "Vertex.h"
#include <vector>
#include <memory>

class ModelResource : public BasicResource
{
public:
	ModelResource(std::string modelName, std::vector<Vertex> verticies, std::vector<uint32_t> indices);
	~ModelResource() override = default;

	std::vector<Vertex> verticies;
	std::vector<uint32_t> indicies;
};
using ModelResourcePtr = std::shared_ptr<ModelResource>;

