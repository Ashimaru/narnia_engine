#include "BasicResource.h"

int BasicResource::nextID = -1;

BasicResource::BasicResource(std::string name) :
	resourceID(++nextID),
	name(std::move(name))
{
}
