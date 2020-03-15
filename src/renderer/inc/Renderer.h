#pragma once
#include "Scene.h"
#include "ResourceManagerAPI.h"
#include "GPU.h"
#include "AbstractRenderModeFactory.h"

class Renderer
{
public:
	Renderer();
	~Renderer() = default;

	bool init(const GPUPtr &gpu, const ResourceManagerAPIPtr &resMan, SimpleRenderMode renderMode);

	void draw();
	void cleanUp();

private:
	bool createSyncObjects();

	size_t m_currentFrameIndex;

	std::vector<vk::Semaphore> m_imageAvailableSemaphores;
	std::vector<vk::Semaphore> m_renderFinishedSemaphores;
	std::vector<vk::Fence> m_frameFences;

	SimpleRenderMode m_renderMode;
	GPUPtr m_gpu;
	ResourceManagerAPIPtr m_resourceManager;
};
using RendererPtr = std::shared_ptr<Renderer>;

