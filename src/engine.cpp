#define NDEBUG

#include <iostream>

#include "renderer/vulkan/sys_vulkan.h"

/// @brief This will be removed but is currently used as a quick and dirty way to test the engine as an exe
/// @return 
int main() {
    //Create an instance of the renderer
    VulkanRenderer* renderer = new VulkanRenderer();

    //Attempt to run the engine catching potentially errors
    try{
        renderer->run();
    } catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
        system("pause");
        delete renderer;
        return EXIT_FAILURE;
    }

    delete renderer;
    system("pause");
    return EXIT_SUCCESS;
}