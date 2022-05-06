#pragma once
#include <iostream>

#include "Camera.h"
#include "GBuffer.h"

#include "SpotLight.h"
#include "VulkanSetup.h"
#include "Model.h"
//#include "DeferredRendering.h"

#include <chrono>

#include "SwapChain.h"
#include "Texture.h"

class ForwardRendering;

class Application
{
public:
    void run();
private:
    //-Initialise all our data for rendering-----------------------------------------------------
    void initVulkan();
    void initScene();
    void prepareRendering();

    //-Initialise glfw window----------------------------------------------------------------------
    void initWindow();

    //-Window/Input callbacks--------------------------------------------------------------------
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    //The main loop----------------------------------------------------------------------------
    void mainLoop();

    //-Per frame functions-------------------------------------------------------------------
    int processKeyInput();
    
    //-End of application cleanup-----------------------------------------------------------
    void cleanup();
private:
    //-Members--------------------------------------------------------------------------------
    GLFWwindow* _window;
    VulkanSetup _vkSetup;
    Camera _camera;
    std::vector<PointLight> _pointLights;
    SpotLight _spotLight;
    Model _model;
    ForwardRendering* _basicRenderer;
    //RenderingBase* _mainRenderer;
    SwapChain _swapChain;

    std::chrono::steady_clock::time_point _prevTime;
    std::chrono::steady_clock::time_point _currTime;
    float _deltaTime;

    bool _framebufferResized=false;
    
};
