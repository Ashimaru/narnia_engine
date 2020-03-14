#include "Application.h"
#include <memory>
#include "LoggerAPI.h"


Application::Application() :
	m_resourceManager{ResourceManagerAPI::createInstance()},
	m_renderEngine{RenderEngineAPI::createInstance()},
	m_isExiting{ false }
{
}

int Application::init()
{
	int result = 0;
	m_resourceManager->LoadResources();
	
	result = m_renderEngine->init(m_resourceManager);
	if (0 != result)
	{
		return result;
	}
	
	return 0;
}

void Application::run()
{
	while (!m_isExiting)
	{
		m_isExiting = m_renderEngine->pollForWindowClose();
		m_renderEngine->drawScene();

	}

	m_renderEngine->waitForRendererToFinish();
}

void Application::cleanUp()
{
	m_renderEngine->cleanUp();
	m_resourceManager->cleanUp();
}
