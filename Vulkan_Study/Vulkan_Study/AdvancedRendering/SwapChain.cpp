#include "SwapChain.h"

#include <stdexcept>

#include "Image.h"
#include "Utils.h"

void SwapChain::initSwapchain(VulkanSetup* pVkSetup)
{
    _vkSetup=pVkSetup;

    createSwapChain();

    //then create the image views for the images created
    _imageViews.resize(_images.size());
    for (size_t i=0;i<_images.size();i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo=utils::initImageViewCreateInfo(_images[i],
            VK_IMAGE_VIEW_TYPE_2D,_imageFormat,{},{VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1});
        _imageViews[i]=VulkanImage::createImageView(_vkSetup,imageViewCreateInfo);
    }
}


void SwapChain::cleanupSwapChain()
{
    //loop over the image views and destroy them. NB we don't destroy the images because they are implicilty created
    //and destroyed by the swap chain
    for (size_t i=0;i<_imageViews.size();i++)
    {
        vkDestroyImageView(_vkSetup->_device,_imageViews[i], nullptr);
    }
    
    vkDestroySwapchainKHR(_vkSetup->_device,_swapChain,nullptr);
}

void SwapChain::createSwapChain()
{
    VulkanSetup::SwapChainSupportDetails supportDetails=_vkSetup->querySwapchainSupport(_vkSetup->_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat=chooseSwapSurfaceFormat(supportDetails.formats);
    VkPresentModeKHR presentMode=chooseSwapPresentMode(supportDetails.presentModes);
    VkExtent2D newExtent=chooseSwapExtent(supportDetails.capabilities);

    uint32_t imageCount = supportDetails.capabilities.minImageCount+1;//+1 to avoid waiting
    if (supportDetails.capabilities.maxImageCount>0&&imageCount>supportDetails.capabilities.maxImageCount)
    {
        imageCount=supportDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType=VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface=_vkSetup->_surface;
    createInfo.minImageCount=imageCount;
    createInfo.imageFormat=surfaceFormat.format;
    createInfo.imageColorSpace=surfaceFormat.colorSpace;
    createInfo.imageExtent=newExtent;
    createInfo.imageArrayLayers=1;
    createInfo.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // how to handle the swapchain images across multiple queue families( in case graphics queue is different to presentation queue)
    utils::QueuefamilyIndices indices=utils::QueuefamilyIndices::findQueueFamilies(_vkSetup->_physicalDevice,_vkSetup->_surface);
    uint32_t queueFamilyIndices[]={indices.graphicsFamily.value(),indices.presentFamily.value()};
    if (indices.graphicsFamily!=indices.presentFamily)
    {
        createInfo.imageSharingMode=VK_SHARING_MODE_CONCURRENT;//Images can be used across multiple queue families without explicit ownership transfers
        createInfo.queueFamilyIndexCount=2;
        createInfo.pQueueFamilyIndices=queueFamilyIndices;
    }else
    {
        //An image is owned by one queue family
        //at a time and ownership must be explicitly transfered before using it in
        //another queue family. This option offers the best performance
        createInfo.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform=supportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode=presentMode;
    createInfo.clipped=VK_TRUE;
    createInfo.oldSwapchain=VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_vkSetup->_device,&createInfo,nullptr,&_swapChain)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    //get images
    vkGetSwapchainImagesKHR(_vkSetup->_device,_swapChain,&imageCount,nullptr);
    _images.resize(imageCount);
    vkGetSwapchainImagesKHR(_vkSetup->_device,_swapChain,&imageCount,_images.data());

    //save format and extent
    _imageFormat=surfaceFormat.format;
    _extent=newExtent;
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    // VkSurfaceFormatKHR entry contains a format and colorSpace member
    // format is colour channels and type eg VK_FORMAT_B8G8R8A8_SRGB (8 bit uint BGRA channels, 32 bits per pixel)
    // colorSpace is the coulour space that indicates if SRGB is supported with VK_COLOR_SPACE_SRGB_NONLINEAR_KHR 
    // (used to be VK_COLORSPACE_SRGB_NONLINEAR_KHR)
    /*************************************************************************************************************/

    //loop through available formats
    for (const auto& availableFormat:availableFormats)
    {
        //if the correct combination of desired format and colour space exists then return the format
        if (availableFormat.format==VK_FORMAT_B8G8R8A8_SRGB&&availableFormat.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    //if above fails, we could rank available formats based on how "good" they are for our task, settle for first element for now
    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    // presentation mode, can be one of four possible values:
    // VK_PRESENT_MODE_IMMEDIATE_KHR -> image submitted by app is sent straight to screen, may result in tearing
    // VK_PRESENT_MODE_FIFO_KHR -> swap chain is a queue where display takes an image from front when display is 
    // refreshed. Program inserts rendered images at back. 
    // If queue full, program has to wait. Most similar vsync. Moment display is refreshed is "vertical blank".
    // VK_PRESENT_MODE_FIFO_RELAXED_KHR -> Mode only differs from previous if application is late and queue empty 
    // at last vertical blank. Instead of waiting for next vertical blank. image is transferred right away when it 
    // finally arrives, may result tearing.
    // VK_PRESENT_MODE_MAILBOX_KHR -> another variation of second mode. Instead of blocking the app when queue is 
    // full, images that are already queued are replaced with newer ones. Can be used to implement triple buffering
    // which allows to avoid tearing with less latency issues than standard vsync using double buffering.   
    /*************************************************************************************************************/

    for (const auto& availablePresentMode:availablePresentModes)
    {
        //use triple buffering if available
        if (availablePresentMode==VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    // swap extent is the resolution of the swap chain images, almost alwawys = to window res we're drawing pixels 
    // in match resolution by setting width and height in currentExtent member of VkSurfaceCapabilitiesKHR struct.
    /*************************************************************************************************************/

    if (capabilities.currentExtent.width!=UINT32_MAX)
    {
        return capabilities.currentExtent;
    }else
    {
        //get the dimensions of the window
        int width,height;
        glfwGetFramebufferSize(_vkSetup->_window,&width,&height);

        //prepare the struct with the height and width of the window
        VkExtent2D actualExtent{static_cast<uint32_t>(width),static_cast<uint32_t>(height)};

        //clamp the values between allowed min and max extents by the surface
        actualExtent.width=std::max(capabilities.minImageExtent.width,std::min(capabilities.maxImageExtent.width,actualExtent.width));
        actualExtent.height=std::max(capabilities.minImageExtent.height,std::min(capabilities.maxImageExtent.height,actualExtent.height));
        
        return actualExtent;
    }
}

/*VulkanSetup::SwapChainSupportDetails SwapChain::querySwapChainSupport() {
    VulkanSetup::SwapChainSupportDetails details;
    // query the surface capabilities and store in a VkSurfaceCapabilities struct
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_vkSetup._physicalDevice, _vkSetup._surface, &details.capabilities); // takes into account device and surface when determining capabilities

    // same as we have seen many times before
    uint32_t formatCount;
    // query the available formats, pass null ptr to just set the count
    vkGetPhysicalDeviceSurfaceFormatsKHR(_vkSetup._physicalDevice, _vkSetup._surface, &formatCount, nullptr);

    // if there are formats
    if (formatCount != 0) {
        // then resize the vector accordingly
        details.formats.resize(formatCount);
        // and set details struct fromats vector with the data pointer
        vkGetPhysicalDeviceSurfaceFormatsKHR(_vkSetup._physicalDevice, _vkSetup._surface, &formatCount, details.formats.data());
    }

    // exact same thing as format for presentation modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_vkSetup._physicalDevice, _vkSetup._surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_vkSetup._physicalDevice, _vkSetup._surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}*/