#include "RenderableObject.h"

RenderableObject::RenderableObject(std::string name, glm::vec3 position) :
	indexCount{0},
	vertexOffset{0},
	m_position{std::move(position)},
	m_name{std::move(name)},
	m_active{true}
{

}

void RenderableObject::updatePosition(glm::vec3 newPosition)
{
	m_position = newPosition;
}


bool RenderableObject::isActive()
{ 
	return m_active;
}
