#pragma once
#include "Renderer.h"
#include "RenderEngineAPI.h"
#include "ResourceManagerAPI.h"
#include "GPU.h"
#include <unordered_map>
#include "SDL2/SDL.h"

class RenderEngine : public RenderEngineAPI
{
public:
	RenderEngine();
	~RenderEngine() override;

	int init(const ResourceManagerAPIPtr& resourceManager) override;
	void drawScene() override;
	bool pollForWindowClose() override;

	void waitForRendererToFinish() override;
	void cleanUp() override;

	RenderableObjectAPIPtr createObject(const std::string &name, const std::string &modelName) override;

	static std::vector<const char *> getValidationLayers();

private:
	bool initSDL();
	bool createSurface();
	bool createInstance();

	std::vector<const char*> getExtensions() const;
	std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages();
	vk::ShaderModule* createShaderModule(const std::vector<char> &code, const std::string &shaderName);

	bool m_isExiting;
	std::unordered_map<std::string, vk::ShaderModule*> m_loadedShaders;
	RendererPtr m_renderer;
	RenderModeFactoryPtr m_renderModeFactory;
	ScenePtr m_scene;
	SDL_Window *m_window;
	vk::Instance m_vulcanInstance;
	vk::SurfaceKHR m_surface;
	GPUPtr m_gpu;
	ResourceManagerAPIPtr m_resourceManager;
};


