#include "Renderer.h"
#include "LoggerAPI.h"
#include <array>

using std::make_unique;
using std::vector;
using std::array;

namespace{
constexpr int NUMBER_OF_FRAMES_IN_FLIGHT = 3;
}

Renderer::Renderer() :
	m_currentFrameIndex(0),
	m_renderMode{},
	m_gpu(nullptr),
	m_resourceManager(nullptr)
{
}

bool Renderer::init(const GPUPtr &gpu, const ResourceManagerAPIPtr &resMan, SimpleRenderMode renderMode)
{
	m_resourceManager = resMan;
	m_gpu = gpu;
	m_renderMode = std::move(renderMode);

	return createSyncObjects();
}

Renderer::~Renderer()
{
	m_gpu->deleteRenderMode(std::move(m_renderMode));

	for (auto i = 0; i < NUMBER_OF_FRAMES_IN_FLIGHT; i++)
	{
		m_gpu->deleteSemaphore(m_renderFinishedSemaphores[i]);
		m_gpu->deleteSemaphore(m_imageAvailableSemaphores[i]);
		m_gpu->deleteFence(m_frameFences[i]);
	}
}

void Renderer::draw()
{
	m_gpu->waitForFence(&m_frameFences[m_currentFrameIndex]);
	m_gpu->resetFence(&m_frameFences[m_currentFrameIndex]);

	uint32_t imageIndex{ 0 };
	m_gpu->acquireNextImage(m_imageAvailableSemaphores[m_currentFrameIndex]);

	vk::Semaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrameIndex] };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput) };
	vk::Semaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrameIndex] };

	const auto submitInfo = vk::SubmitInfo{
		1,
		waitSemaphores,
		waitStages,
		1, 
		&m_renderMode.commandBuffers[m_currentFrameIndex],
		1, signalSemaphores
	};

	m_gpu->submitToGraphicsQueue(&submitInfo, m_frameFences[m_currentFrameIndex]);

	auto presentInfo = vk::PresentInfoKHR();
	presentInfo.setWaitSemaphoreCount(1);
	presentInfo.setPWaitSemaphores(signalSemaphores);
	presentInfo.setPSwapchains(&m_gpu->swapchain);
	presentInfo.setSwapchainCount(1);
	presentInfo.setPImageIndices(&imageIndex);

	m_gpu->submitToPresentationQueue(presentInfo);

	m_currentFrameIndex = ++m_currentFrameIndex % NUMBER_OF_FRAMES_IN_FLIGHT;

}

bool Renderer::createSyncObjects()
{
	m_imageAvailableSemaphores.resize(NUMBER_OF_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(NUMBER_OF_FRAMES_IN_FLIGHT);
	m_frameFences.resize(NUMBER_OF_FRAMES_IN_FLIGHT);


	auto semaphoreInfo = vk::SemaphoreCreateInfo();
	auto fenceInfo = vk::FenceCreateInfo();
	fenceInfo.setFlags(vk::FenceCreateFlags(vk::FenceCreateFlagBits::eSignaled));

	for (auto i = 0; i < NUMBER_OF_FRAMES_IN_FLIGHT; ++i)
	{
		m_gpu->createSemaphore(semaphoreInfo, m_imageAvailableSemaphores[i]);
		m_gpu->createSemaphore(semaphoreInfo, m_renderFinishedSemaphores[i]);
		m_gpu->createFence(fenceInfo, m_frameFences[i]);
	}

	return true;
}