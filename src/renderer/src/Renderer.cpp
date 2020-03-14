#include "Renderer.h"
#include "LoggerAPI.h"
#include <array>

using std::make_unique;
using std::vector;
using std::array;

const int FramesInFlightCount = 3;

Renderer::Renderer() :
	m_currentFrameIndex(0),
	m_renderMode(nullptr),
	m_gpu(nullptr),
	m_resourceManager(nullptr)
{
}

bool Renderer::init(const GPUPtr &gpu, const ResourceManagerAPIPtr &resMan, RenderModePtr renderMode)
{
	m_resourceManager = resMan;
	m_gpu = gpu;
	m_renderMode = std::move(renderMode);

	return createSyncObjects();
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
		m_renderMode->getCommandBuffer(m_currentFrameIndex),
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

	m_currentFrameIndex = ++m_currentFrameIndex % FramesInFlightCount;

}

void Renderer::cleanUp()
{
	m_renderMode->cleanUp();

	for (auto i = 0; i < FramesInFlightCount; i++)
	{
		m_gpu->deleteSemaphore(m_renderFinishedSemaphores[i]);
		m_gpu->deleteSemaphore(m_imageAvailableSemaphores[i]);
		m_gpu->deleteFence(m_frameFences[i]);
	}
}

bool Renderer::createSyncObjects()
{
	m_imageAvailableSemaphores.resize(FramesInFlightCount);
	m_renderFinishedSemaphores.resize(FramesInFlightCount);
	m_frameFences.resize(FramesInFlightCount);


	auto semaphoreInfo = vk::SemaphoreCreateInfo();
	auto fenceInfo = vk::FenceCreateInfo();
	fenceInfo.setFlags(vk::FenceCreateFlags(vk::FenceCreateFlagBits::eSignaled));

	for (auto i = 0; i < FramesInFlightCount; ++i)
	{
		m_gpu->createSemaphore(semaphoreInfo, m_imageAvailableSemaphores[i]);
		m_gpu->createSemaphore(semaphoreInfo, m_renderFinishedSemaphores[i]);
		m_gpu->createFence(fenceInfo, m_frameFences[i]);
	}

	return true;
}