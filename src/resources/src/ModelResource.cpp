#include "ModelResource.h"

ModelResource::ModelResource(std::string modelName, std::vector<Vertex> verticies, std::vector<uint32_t> indices) :
	BasicResource(std::move(modelName)),
	verticies(std::move(verticies)),
	indicies(std::move(indices))
{
}
