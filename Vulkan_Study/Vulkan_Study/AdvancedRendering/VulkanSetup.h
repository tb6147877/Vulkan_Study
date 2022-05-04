#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class VulkanSetup
{
public:
    //----------------------------------------------------------------------------------------
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR        capabilities;//surface的基础功能（swap chain中的图片数量和大小）
        std::vector<VkSurfaceFormatKHR> formats;//surface中图片的格式和颜色空间
        std::vector<VkPresentModeKHR>   presentModes;//可用的表现模式
    };

public:
    //-Initialisation and cleanup-------------------------------------------------------------
    void initSetup(GLFWwindow* window);
    void cleanupSetup();

    //-Swap chain related--------------------------------------------------------------------------
    SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice device);

    //-Physical device limit query--------------------------------------------------------
    VkBool32 isUniformBufferOffsetValid(VkDeviceSize size);
    

public:
    //-Members------------------------------------------------------------------------
    GLFWwindow* _window;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;

    VkSurfaceKHR _surface;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    VkPhysicalDeviceProperties _deviceProperties;

    bool _setupComplete=false;

private:
    //-Vulkan instance--------------------------------------------------------
    void createInstance();
    std::vector<const char*> getRequiredExtensions();//extensions required for instance


    //-Validation layers-------------------------------------------------------------------
    void setupDebugMessenger();
    static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator,VkDebugUtilsMessengerEXT* pDebugMessenger);
    static void destroyDebugUtilsMessengerEXT(VkInstance instance,const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT debugMessenger);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool checkValidationLayerSupport();

    //-Vukan surface (window)---------------------------------------------------------------------
    void createSurface();

    //-Vulkan devices---------------------------------------------------------------------------------
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void createLogicalDevice();
};

