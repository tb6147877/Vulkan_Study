#pragma once
#include <iostream>

#include "Camera.h"
#include "GBuffer.h"

#include "SpotLight.h"
#include "VulkanSetup.h"
#include "Model.h"
#include "DeferredRendering.h"
#include "SwapChain.h"

class Application
{
public:
    void run();
private:
    //-Initialise all our data for rendering-----------------------------------------------------
    void initVulkan();
    void initScene();

    //-Initialise glfw window----------------------------------------------------------------------
    void initWindow();

    //-Window/Input callbacks--------------------------------------------------------------------
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);


    //-End of application cleanup-----------------------------------------------------------
    void cleanup();
private:
    //-Members--------------------------------------------------------------------------------
    GLFWwindow* _window;
    VulkanSetup _vkSetup;
    Camera _camera;
    PointLight _pointLights[1];
    SpotLight _spotLight;
    Model _model;
    RenderingBase* _renderer;
    SwapChain _swapChain;

    VkCommandPool _renderCommandPool;
    std::vector<VkCommandBuffer> _offScreenCommandBuffer;
    std::vector<VkCommandBuffer> _renderCommandBuffer;
    std::vector<VkCommandBuffer> _shadowMapCommandBuffer;
    

    bool _framebufferResized=false;
    
};
