add_subdirectory(main)
add_subdirectory(resources)
add_subdirectory(renderer)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(
  ${PROJECT_NAME} PUBLIC project_options project_warnings main resourceManagement renderer)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/shaders/compileShaders.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/bin/Shaders/)