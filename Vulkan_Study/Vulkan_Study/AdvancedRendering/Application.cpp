#include "Application.h"

#include <GLFW/glfw3.h>

#include "AppConstants.h"
#include "ForwardRendering.h"
#include "Utils.h"



void Application::run()
{
    initWindow();
    initVulkan();
    initScene();
    
}

void Application::cleanup()
{
    _basicRenderer->cleanupRenderer();
    delete _basicRenderer;
    _swapChain.cleanupSwapChain();
    _vkSetup.cleanupSetup();


    
    //destroy the window
    glfwDestroyWindow(_window);
    //terminate glfw
    glfwTerminate();
}

void Application::initVulkan()
{
    _vkSetup.initSetup(_window);
    _swapChain.initSwapchain(&_vkSetup);
    _basicRenderer = new ForwardRendering();
    _basicRenderer->initRenderer(&_vkSetup,&_swapChain,&_model);
   
    
}

void Application::initScene()
{
    _camera=Camera({0.0f,0.0f,0.0f},2.0f,10.0f);
    _model.loadModel(MODEL_PATH);
    _model.loadModelTextures(&_vkSetup,_basicRenderer->_renderCommandPool,MODEL_TEXTURES_PATH);
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
