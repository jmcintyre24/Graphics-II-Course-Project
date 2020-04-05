cmake_minimum_required(VERSION 3.16)

project(DEV4_Project)

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_executable (Project main.cpp DDSTextureLoader.cpp DDSTextureLoader.h defines.h DrawClass.h main.cpp StoneHenge.h)
target_link_libraries(Project d3d11.lib d3dcompiler.lib)

file(COPY ".\\Textures\\StoneHenge.dds" DESTINATION Textures)
file(COPY ".\\Shaders\\shaders.fx" DESTINATION Shaders)
file(COPY ".\\Shaders\\DEV4_PS.hlsl" DESTINATION Shaders)
file(COPY ".\\Shaders\\DEV4_VS.hlsl" DESTINATION Shaders)