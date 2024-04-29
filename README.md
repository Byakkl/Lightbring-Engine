# Lightbring-Engine
Long term project to build a basic game engine

This is primarily a self-teaching project to learn a variety of tools and SDKs while improving my c++ skills

## Current Progress
* The project now compiles down into a library with public headers for implementation and testing is being done in a separate project
* Basic Vulkan pipeline setup is complete with basic OBJ mesh and single texture (albedo) support
* Initial layout conversion of Vulkan tutorial to engine architecture is done allowing quad primitive creation and texture import via engine calls
* CMake setup is functional and connects properly with VSCode


## Goals
* Learning more about engine structure and design as well as various c++ design patterns to ensure cleaner code
* Learn Vulkan by following https://vulkan-tutorial.com to set up basic rendering
* Learn CMake and more details of the build process

## Notes for compiling and using
Compilation is done with MinGW/g++ using CMake as the build system

Because I've been using this as a way to learn CMake I opted to not include pre-compiled libraries of GLFW and GLM.
In order to compile the project the CMakeLists.txt file will need to be updated to point to your local copies of the libraries

After building the engine include the LightbringEngine library files (.dll and .lib) in your project as well as all of the files in the "include" folder
    The output directory for the build is set to "build/output"
GLM is an external dependency and will need to be included in the implementing project. 
At this time I don't have support for custom shaders but the .spv files in "include/shaders" directory are usable. 
    Make sure to inform the engine of where they are by using engine->setVertexShaderPath() and engine->setFragmentShaderPath()

## Included libraries and versions
* Vulkan SDK v1.3.275.0
* GLFW v3.4
* GLM v1.0.0
* stb_image.h (https://github.com/nothings/stb)
* TinyOBJLoader (https://github.com/tinyobjloader/tinyobjloader)