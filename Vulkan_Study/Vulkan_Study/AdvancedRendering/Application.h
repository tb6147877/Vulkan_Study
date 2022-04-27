#pragma once
#include <iostream>

#include "Camera.h"
#include "GBuffer.h"
#include "SpotLight.h"
#include "VulkanSetup.h"


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

    //-Command buffer initialisation functions-------------------------------------------
    void createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags);

    //-Window/Input callbacks--------------------------------------------------------------------
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
    //-Members--------------------------------------------------------------------------------
    GLFWwindow* _window;
    VulkanSetup _vkSetup;
    Camera _camera;
    PointLight _pointLights[1];
    SpotLight _spotLight;

    VkCommandPool _renderCommandPool;
    std::vector<VkCommandBuffer> _offScreenCommandBuffer;
    std::vector<VkCommandBuffer> _renderCommandBuffer;
    std::vector<VkCommandBuffer> _shadowMapCommandBuffer;
    

    bool _framebufferResized=false;
    
};
