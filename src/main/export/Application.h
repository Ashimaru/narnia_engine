#pragma once
#include "ResourceManagerAPI.h"
#include "RenderEngineAPI.h"


class Application
{
public:
	Application();
	~Application();

	void run();
private:

	ResourceManagerAPIPtr m_resourceManager;
	RenderEngineAPIPtr m_renderEngine;
	bool m_isExiting;
};

