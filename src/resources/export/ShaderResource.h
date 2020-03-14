#pragma once
#include "BasicResource.h"
#include <vector>

class ShaderResource : public BasicResource
{
public:
	ShaderResource(const std::string &name, std::vector<char> shaderCode);
	~ShaderResource() override = default;

	std::vector<char> shader;
};

