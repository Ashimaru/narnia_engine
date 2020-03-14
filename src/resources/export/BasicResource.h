#pragma once
#include <string>

class BasicResource
{
public:
	explicit BasicResource(std::string name);
	virtual ~BasicResource() = default;
	int resourceID;
	std::string name;

private:
	static int nextID;
};

