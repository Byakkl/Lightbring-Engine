# Lightbring-Engine
Long term project to build a basic game engine

This is primarily a self-teaching project to learn a variety of tools and SDKs while improving my c++ skills

## Current Progress
* WIP is OBJ import support, having the engine compile into a single library and moving the engine main function into its own project that implements the engine library
* Basic Vulkan pipeline setup is complete with basic mesh and single texture support
* Initial layout conversion of Vulkan tutorial to engine architecture is done allowing quad primitive creation and texture import via engine calls
* CMake setup is functional and connects properly with VSCode


## Goals
* Learn Vulkan by following https://vulkan-tutorial.com to set up basic rendering
* Learn CMake and more details of the build process

## Notes for compiling
Compilation is done with MinGW/g++ using CMake as the build system

Because I've been using this as a way to learn CMake I opted to not include pre-compiled libraries of GLFW and GLM.
In order to compile the project the CMakeLists.txt file will need to be updated to point to your local copies of the libraries
I've also left out the compiled shaders and just kept the base files with the batch file used to generate the .spv results. The batch file will need to be edited to point to the Vulkan SDK but should otherwise be functional

## Included libraries and versions
* Vulkan SDK v1.3.275.0
* GLFW v3.4
* GLM v1.0.0
* stb_image.h (https://github.com/nothings/stb)
* TinyOBJLoader (https://github.com/tinyobjloader/tinyobjloader)