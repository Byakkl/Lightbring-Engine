#pragma once

#include <GLFW/glfw3.h>

class Input_Internal{
public:
    void initialize(GLFWwindow* a_window){
        glfwSetKeyCallback(a_window, keyCallback);
    }

    static void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods){
        
    }

    static void cursorPositionCallback(GLFWwindow* a_window, double a_xPos, double a_yPos){

    }

    static void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods){

    }

    static void scrollCallback(GLFWwindow* a_window, double a_xOffset, double a_yOffset){

    }
};