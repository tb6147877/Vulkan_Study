#pragma once

#include "VulkanSetup.h"

//A class that contains the swapchain setup and data. It has a create function to facilitate the recreation
//when a window is resized. 

class SwapChain
{
public:
    //-Initialisation and cleanup------------------------------------------------------------
    void initSwapchain(VulkanSetup* pVkSetup);
    void cleanupSwapChain();

private:
    //-Swapchain creation helpers----------------------------------------------------------
    void createSwapChain();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

public:
    //-Members-------------------------------------------------------------------------------
    VulkanSetup* _vkSetup;
    
    VkSwapchainKHR _swapChain;
    
    VkExtent2D _extent;
    VkFormat _imageFormat;
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imageViews;

};
