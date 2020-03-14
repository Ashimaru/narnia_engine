#include "ShaderResource.h"


ShaderResource::ShaderResource(const std::string &name, std::vector<char> shaderCode) :
	BasicResource(name),
	shader{std::move(shaderCode)}
{
}