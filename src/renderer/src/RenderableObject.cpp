#include "RenderableObject.h"

RenderableObject::RenderableObject(std::string name) :
	indexCount{0},
	vertexOffset{0},
	m_name{std::move(name)},
	m_active{true}
{

}

bool RenderableObject::isActive()
{ 
	return m_active;
}
