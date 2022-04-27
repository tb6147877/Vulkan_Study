#include "Application.h"

#include <GLFW/glfw3.h>

#include "AppConstants.h"
#include "Utils.h"

void Application::run()
{
    initWindow();
    initScene();
    initVulkan();
}

void Application::initVulkan()
{
    _vkSetup.initSetup(_window);
   
    createCommandPool(&_renderCommandPool,VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    
}

void Application::initScene()
{
    _camera=Camera({0.0f,0.0f,0.0f},2.0f,10.0f);
    _pointLights[0]= { {5.0f, -5.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f, 40.0f} };
    _spotLight={{5.0f,-5.0f,0.0f},0.1f,40.0f};
}

void Application::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);//initially for opengl, so tell it not to create opengl context
    _window=glfwCreateWindow(WIDTH,HEIGHT,"Vulkan Advanced Rendering",nullptr,nullptr);
    glfwSetWindowUserPointer(_window,this);//set a user pointer to a window
    glfwSetFramebufferSizeCallback(_window,framebufferResizeCallback);
}

void Application::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    //pointer to this application class obtained from glfw, it doesn't know that it is a Application but we do so we can cast to it
    auto app=reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    app->_framebufferResized=true;
}

void Application::createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags)
{
    utils::QueuefamilyIndices queueFamilyIndices=utils::QueuefamilyIndices::findQueueFamilies(_vkSetup._physicalDevice,_vkSetup._surface);
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex=queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags=flags;

    if (vkCreateCommandPool(_vkSetup._device,&poolInfo,nullptr,commandPool)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}