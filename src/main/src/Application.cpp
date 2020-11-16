#include "Application.h"
#include <memory>
#include "LoggerAPI.h"


Application::Application() :
	m_resourceManager{ResourceManagerAPI::createInstance()},
	m_renderEngine{RenderEngineAPI::createInstance()},
	m_isExiting{ false }
{
	m_resourceManager->LoadResources();
	assert(m_renderEngine->init(m_resourceManager));
}

void Application::run()
{
	m_renderEngine->createObject("DUMMY", "rectangle");
	m_renderEngine->createObject("TestTriangle", "triangle");

	while (!m_isExiting)
	{
		m_isExiting = m_renderEngine->pollForWindowClose();
		m_renderEngine->drawScene();
	}

	m_renderEngine->waitForRendererToFinish();
}

Application::~Application()
{
	m_renderEngine->cleanUp();
	m_resourceManager->cleanUp();
}
