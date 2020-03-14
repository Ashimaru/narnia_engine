add_library(renderer STATIC 
            GPU.cpp
            GPUFactory.cpp
            RenderableObject.cpp
            RenderEngine.cpp
            Renderer.cpp
            Scene.cpp
            SimpleRenderMode.cpp
            SimpleRenderModeFactory.cpp
)

find_package(Vulkan REQUIRED)

target_include_directories(renderer PRIVATE ../inc Vulkan::Vulkan)
target_include_directories(renderer PUBLIC ../export)

target_link_libraries(
    renderer PUBLIC project_options project_warnings main resourceManagement CONAN_PKG::glm CONAN_PKG::sdl2 PRIVATE Vulkan::Vulkan)
