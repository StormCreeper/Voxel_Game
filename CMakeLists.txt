cmake_minimum_required(VERSION 3.5)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED True)

project(tpOpenGL)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SOURCES
  main.cpp
  gl_objects/mesh.cpp
  gl_objects/shader.cpp
  gl_objects/texture.cpp
  world_builder.cpp
  chunks/chunk.cpp
  chunks/chunk_manager.cpp
  chunks/chunk_dealer.cpp
  SimplexNoise.cpp

  utils/gl_includes.hpp
  utils/debug.hpp
  gl_objects/mesh.hpp
  gl_objects/shader.hpp
  gl_objects/texture.hpp
  chunks/chunk.hpp
  chunks/chunk_manager.hpp
  chunks/chunk_dealer.hpp
  world_builder.hpp
  block_palette.hpp
  camera.hpp
  SimplexNoise.h
  player.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_sources(${PROJECT_NAME} PRIVATE dep/glad/src/glad.c)
target_include_directories(${PROJECT_NAME} PRIVATE dep/glad/include/)

add_subdirectory(dep/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

add_subdirectory(dep/glm)
target_link_libraries(${PROJECT_NAME} glm)

target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})

# Create a custom target to copy resources to the build directory (Added by Telo PHILIPPE)
add_custom_target(CopyResources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
)