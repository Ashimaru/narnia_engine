#include "RenderEngine.h"
#include "GPUFactory.h"
#include "LoggerAPI.h"
#include "SimpleRenderModeFactory.h"
#include "SDL2/SDL_vulkan.h"
#include <fmt/core.h>

#pragma warning(disable : 4201)
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#pragma warning(default : 4201)

using vk::ApplicationInfo;
using vk::InstanceCreateInfo;
using vk::Instance;

using std::vector;

namespace {
constexpr int XResolution = 1280;
constexpr int YResolution = 720;

vk::ApplicationInfo getApplicationInfo()
{
  // vk::ApplicationInfo allows the programmer to specifiy some basic information about the
  // program, which can be useful for layers and tools to provide more debug information.
  vk::ApplicationInfo result;
  result.setPApplicationName("Vulcan Renderer").setApplicationVersion(1).setPEngineName("VulcanRenderer").setEngineVersion(1).setApiVersion(VK_API_VERSION_1_0);

  return result;
}
}// namespace

RenderEngineAPIPtr RenderEngineAPI::createInstance()
{
  return RenderEngineAPIPtr(new RenderEngine());
}

RenderEngine::RenderEngine() : m_isExiting(false),
                               m_renderer(std::make_shared<Renderer>()),
                               m_scene{ std::make_shared<Scene>() },
                               m_window(nullptr),
                               m_resourceManager(nullptr)
{
}


RenderEngine::~RenderEngine()
{
}

int RenderEngine::init(const ResourceManagerAPIPtr &resourceManager)
{
  m_resourceManager = resourceManager;

  if (!initSDL())
    return 1;

  if (!createInstance())
    return 2;

  if (!createSurface())
    return 3;

  m_gpu = GPUFactory::createGPU(m_vulcanInstance, m_surface, getValidationLayers());
  m_renderModeFactory = std::make_unique<SimpleRenderModeFactory>(m_gpu, m_scene);

  auto shaders = createShaderStages();
  auto swapchainFormat = m_gpu->getSwapchanFormat();
  auto viewportExtent = m_gpu->getPresentationExtent();

  auto result = m_renderer->init(m_gpu, resourceManager, m_renderModeFactory->createRenderMode(swapchainFormat, viewportExtent, shaders));

  if (!result)
    return 4;

  return 0;
}

void RenderEngine::drawScene()
{
  m_renderer->draw();
}

void RenderEngine::waitForRendererToFinish()
{
  m_gpu->waitForRender();
}

void RenderEngine::cleanUp()
{
  for (auto &object : m_scene->renderableObjects) {
    m_gpu->unloadROFromMemory(object);
  }

  for (auto shaderModule : m_loadedShaders) {
    m_gpu->deleteShaderModule(*shaderModule.second);
  }
  m_loadedShaders.clear();

  m_resourceManager->cleanUp();
  m_gpu->cleanUp();
  m_vulcanInstance.destroySurfaceKHR(m_surface);
  m_vulcanInstance.destroy();

  SDL_DestroyWindow(m_window);
  SDL_Quit();
}

bool RenderEngine::pollForWindowClose()
{
  SDL_Event sdlEvent;
  while (SDL_PollEvent(&sdlEvent)) {

    switch (sdlEvent.type) {

    case SDL_QUIT:
      return true;
      break;

    default:
      // Do nothing.
      break;
    }
  }
  return false;
}

RenderableObjectAPIPtr RenderEngine::createObject(std::string name, const std::string &modelName)
{
  LoggerAPI::getLogger()->logInfo(fmt::format("Creating object {} with model {}", name, modelName));
  auto model = m_resourceManager->getModel(std::move(modelName));
  auto object = std::make_shared<RenderableObject>(name);

  object->indexCount = static_cast<uint32_t>(model.indicies.size());
  object->vertexOffset = model.verticies.size() * sizeof(Vertex);

  m_gpu->loadROToMemory(model, object);

  m_scene->renderableObjects.emplace_back(object);

  return object;
}

RenderableObjectAPIPtr RenderEngine::createObject(std::string name, const std::string &modelName, glm::vec3 position)
{
  LoggerAPI::getLogger()->logInfo(fmt::format("Creating object {} with model {} at ", name, modelName, glm::to_string(position)));
  auto model = m_resourceManager->getModel(modelName);
  auto object = std::make_shared<RenderableObject>(std::move(name), std::move(position));

  object->indexCount = static_cast<uint32_t>(model.indicies.size());
  object->vertexOffset = model.verticies.size() * sizeof(Vertex);

  m_gpu->loadROToMemory(model, object);

  m_scene->renderableObjects.emplace_back(object);

  return object;
}

bool RenderEngine::initSDL()
{
  // Create an SDL window that supports Vulkan rendering.
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    LoggerAPI::getLogger()->logCritical("Could not initialize SDL.");
    return false;
  }
  m_window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, XResolution, YResolution, SDL_WINDOW_VULKAN);
  if (m_window == nullptr) {
    LoggerAPI::getLogger()->logCritical("Could not create SDL window.");
    return false;
  }

  return true;
}

bool RenderEngine::createSurface()
{
  // Create a Vulkan surface for rendering
  VkSurfaceKHR c_surface = VK_NULL_HANDLE;
  if (SDL_FALSE == SDL_Vulkan_CreateSurface(m_window, m_vulcanInstance, &c_surface)) {
    LoggerAPI::getLogger()->logCritical("Could not create a Vulkan surface.");
    return false;
  }
  m_surface = vk::SurfaceKHR(c_surface);
  return true;
}

bool RenderEngine::createInstance()
{
  auto extensions = getExtensions();
  auto layers = getValidationLayers();
  auto appInfo = getApplicationInfo();

  vk::InstanceCreateInfo instInfo = {};
  instInfo.setPApplicationInfo(&appInfo);
  instInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
  instInfo.setPpEnabledExtensionNames(extensions.data());
  instInfo.setEnabledLayerCount(static_cast<uint32_t>(layers.size()));
  instInfo.setPpEnabledLayerNames(layers.data());

  // Create the Vulkan instance.
  try {
    vk::createInstance(&instInfo, nullptr, &this->m_vulcanInstance);
  } catch (const std::exception &e) {
    LoggerAPI::getLogger()->logCritical("Could not create a Vulkan instance: " + std::string(e.what()));
    return false;
  }

  return true;
}

vector<const char *> RenderEngine::getValidationLayers()
{
  auto layers = vector<const char *>();

#ifdef _DEBUG
  layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

  //Check if validation is supported
  uint32_t layersCount = { 0 };
  vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
  auto layerProperties = std::vector<VkLayerProperties>(layersCount);

  vkEnumerateInstanceLayerProperties(&layersCount, layerProperties.data());

  auto logger = LoggerAPI::getLogger();


  for (auto it = std::begin(layers); it != std::end(layers);) {
    bool layerFound = false;
    logger->logInfo(*it);
    for (auto layerProperty : layerProperties) {
      if (strcmp(layerProperty.layerName, *it)) {
        layerFound = true;
        break;
      }
    }
    if (!layerFound) {
      logger->logError("Debug Layer not supported");
      it = layers.erase(it);
    } else {
      ++it;
    }
  }

  return layers;
}

vector<const char *> RenderEngine::getExtensions() const
{
  auto extensions = vector<const char *>();
  auto logger = LoggerAPI::getLogger();
  // Get WSI extensions from SDL (we can add more if we like - we just can't remove these)
  unsigned extension_count;
  if (!SDL_Vulkan_GetInstanceExtensions(m_window, &extension_count, NULL)) {
    logger->logError("Could not get the number of required instance extensions from SDL.");
    return extensions;
  }
  extensions.resize(extension_count);
  if (!SDL_Vulkan_GetInstanceExtensions(m_window, &extension_count, extensions.data())) {
    logger->logError("Could not get the names of required instance extensions from SDL.");
    return extensions;
  }

  for (const char *extension : extensions) {
    logger->logInfo(extension);
  }

  return extensions;
}

vector<vk::PipelineShaderStageCreateInfo> RenderEngine::createShaderStages()
{
  auto result = vector<vk::PipelineShaderStageCreateInfo>();

  const std::vector<char> &vertShaderCode = m_resourceManager->getShader("vert").shader;
  auto vertShaderModule = createShaderModule(vertShaderCode, "vert");
  auto vertShaderInfo = vk::PipelineShaderStageCreateInfo();
  vertShaderInfo.setModule(*vertShaderModule);
  vertShaderInfo.setPName("main");
  vertShaderInfo.setStage(vk::ShaderStageFlagBits::eVertex);

  const std::vector<char> &fragShaderCode = m_resourceManager->getShader("frag").shader;
  auto fragShaderModule = createShaderModule(fragShaderCode, "frag");
  auto fragShaderInfo = vk::PipelineShaderStageCreateInfo();
  fragShaderInfo.setModule(*fragShaderModule);
  fragShaderInfo.setPName("main");
  fragShaderInfo.setStage(vk::ShaderStageFlagBits::eFragment);

  result.push_back(vertShaderInfo);
  result.push_back(fragShaderInfo);

  return result;
}

vk::ShaderModule *RenderEngine::createShaderModule(const std::vector<char> &code, const std::string &shaderName)
{
  auto shaderCreateInfo = vk::ShaderModuleCreateInfo{};
  shaderCreateInfo.setCodeSize(code.size());
  shaderCreateInfo.setPCode(reinterpret_cast<const uint32_t *>(code.data()));

  auto shaderModule = new vk::ShaderModule();
  m_gpu->createShaderModule(shaderCreateInfo, *shaderModule);
  m_loadedShaders.insert_or_assign(shaderName, shaderModule);

  return shaderModule;
}
