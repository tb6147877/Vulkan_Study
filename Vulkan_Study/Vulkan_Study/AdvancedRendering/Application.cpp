#include "Application.h"

#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // because OpenGL uses depth range -1.0-1.0 and Vulkan uses 0.0-1.0
#include "AppConstants.h"
#include "ForwardRendering.h"
#include "Utils.h"



void Application::run()
{
    initWindow();
    initVulkan();
    initScene();
    prepareRendering();
    mainLoop();
    //cleanup();
}

void Application::mainLoop()
{
    _prevTime=std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
        _currTime=std::chrono::high_resolution_clock::now();
        _deltaTime=std::chrono::duration<float, std::chrono::seconds::period>(_currTime-_prevTime).count();
        if (processKeyInput()==0)
        {
            break;
        }
        _basicRenderer->drawFrame();
        _prevTime=_currTime;
    }
    vkDeviceWaitIdle(_vkSetup._device);
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
    _camera=Camera(glm::vec3{0.0f,0.0f,0.0f}, glm::vec3{ 0.0f,0.0f,1.0f },0,0);
    _model.loadModel(MODEL_PATH);
    _model.loadModelTextures(&_vkSetup,_basicRenderer->_renderCommandPool,MODEL_TEXTURES_PATH);
    _model.generateModelVertexBuffer(&_vkSetup,_basicRenderer->_renderCommandPool);
    _pointLights[0]= { {5.0f, -5.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f, 40.0f} };
    _spotLight={{5.0f,-5.0f,0.0f},0.1f,40.0f};
}

void Application::prepareRendering()
{
    _basicRenderer->createDescriptorSets();
    _basicRenderer->recordCommandBuffers();
    _basicRenderer->SetCamera(&_camera);
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

int Application::processKeyInput()
{
    if (glfwGetKey(_window,GLFW_KEY_ESCAPE))
    {
        return 0;
    }

    if (glfwGetKey(_window,GLFW_KEY_W))
    {
        _camera.processKeyboard(Camera::MovementType::Forward,_deltaTime);
    }

    if (glfwGetKey(_window,GLFW_KEY_S))
    {
        _camera.processKeyboard(Camera::MovementType::Back,_deltaTime);
    }

    if (glfwGetKey(_window,GLFW_KEY_A))
    {
        _camera.processKeyboard(Camera::MovementType::Left,_deltaTime);
    }

    if (glfwGetKey(_window,GLFW_KEY_D))
    {
        _camera.processKeyboard(Camera::MovementType::Right,_deltaTime);
    }

    if (glfwGetKey(_window,GLFW_KEY_Q))
    {
        _camera.processKeyboard(Camera::MovementType::Up,_deltaTime);
    }

    if (glfwGetKey(_window,GLFW_KEY_E))
    {
        _camera.processKeyboard(Camera::MovementType::Down,_deltaTime);
    }

    if (glfwGetKey(_window,GLFW_KEY_RIGHT))
    {
        _camera.processMouseMovement(0,-0.3,true);
    }

    if (glfwGetKey(_window,GLFW_KEY_LEFT))
    {
        _camera.processMouseMovement(0,0.3,true);
    }

    if (glfwGetKey(_window,GLFW_KEY_UP))
    {
        _camera.processMouseMovement(0.3,0,true);
    }

    if (glfwGetKey(_window,GLFW_KEY_DOWN))
    {
        _camera.processMouseMovement(-0.3,0,true);
    }
    return 1;
}
