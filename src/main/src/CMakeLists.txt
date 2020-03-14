add_library(main STATIC Application.cpp Logger.cpp)

target_include_directories(main PUBLIC ../export PRIVATE ../inc)
target_link_libraries(main PUBLIC project_options project_warnings PRIVATE CONAN_PKG::spdlog renderer resourceManagement)