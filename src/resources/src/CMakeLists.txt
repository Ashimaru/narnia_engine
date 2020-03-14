add_library(resourceManagement STATIC 
            BasicResource.cpp
            FileHelper.cpp
            ModelResource.cpp
            ResourceManager.cpp
            ShaderResource.cpp
)

target_include_directories(resourceManagement PRIVATE ../inc)
target_include_directories(resourceManagement PUBLIC ../export)

target_link_libraries(
    resourceManagement PUBLIC project_options project_warnings main renderer)
