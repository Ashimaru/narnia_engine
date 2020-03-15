#pragma once
#include <vector>
#include "Vertex.h"


struct ModelData
{
    ModelData(const std::vector<Vertex>& verts, const std::vector<std::uint32_t>& indcs, std::uint32_t& usgCounter) :
		verticies{verts},
		indicies{indcs},
		usageCounter{usgCounter}
	{
		++usageCounter;
	}

	ModelData(const ModelData&) = delete;
	ModelData operator= (const ModelData&) = delete;

	inline ~ModelData()
	{
		--usageCounter;
	}

	const std::vector<Vertex>& verticies;
	const std::vector<std::uint32_t>& indicies;
private:
	std::uint32_t usageCounter;
};