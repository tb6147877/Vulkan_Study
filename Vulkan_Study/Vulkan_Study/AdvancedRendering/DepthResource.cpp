#include "DepthResource.h"

#include <stdexcept>

#include "Utils.h"


void DepthResource::createDepthResource(VulkanSetup* vkSetup, const VkExtent2D& extent, VkCommandPool commandPool)
{
    _vkSetup=vkSetup;
    //depth image should have the same resolution as the color attachment, defined by swapchain extent
    VkFormat depthFormat=findDepthFormat(_vkSetup);

    //we have the information needed to create an image(the format,usage etc) and image view
    VulkanImage::ImageCreateInfo info{};
    info.width=extent.width;
    info.height=extent.height;
    info.format=depthFormat;
    info.tiling=VK_IMAGE_TILING_OPTIMAL;
    info.usage=VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    info.properties=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    info.pVulkanImage=&_depthImage;

    VulkanImage::createImage(_vkSetup,commandPool,info);

    VkImageViewCreateInfo imageViewCreateInfo=utils::initImageViewCreateInfo(_depthImage._image,VK_IMAGE_VIEW_TYPE_2D,
        depthFormat,{},{VK_IMAGE_ASPECT_DEPTH_BIT,0,1,0,1});
    _depthImageView=VulkanImage::createImageView(_vkSetup,imageViewCreateInfo);

    VulkanImage::LayoutTransitionInfo transitionInfo{};
    transitionInfo.pVulkanImage=&_depthImage;
    transitionInfo.renderCommandPool=commandPool;
    transitionInfo.format=depthFormat;
    transitionInfo.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    transitionInfo.newLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //undefined layout is used as the initial layout as there are no existing depth image contents that matter
    VulkanImage::transitionImageLayout(_vkSetup,transitionInfo);
}


void DepthResource::cleanDepthResource()
{
    vkDestroyImageView(_vkSetup->_device,_depthImageView,nullptr);
    _depthImage.cleanupImage(_vkSetup);
}

VkFormat DepthResource::findDepthFormat(const VulkanSetup* vkSetup)
{
    //return a certain depth format if available
    return findSupportedFormat(
        vkSetup,
        {VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat DepthResource::findSupportedFormat(const VulkanSetup* vkSetup, const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features)
{
    //instead of a fixed format, get a list of formats ranked from most to least  desirable and iterate through it
    for (VkFormat format:candidates)
    {
        //query the support of the format by the device
        VkFormatProperties props;// contains three fields
        // linearTilingFeatures: Use cases that are supported with linear tiling
        // optimalTilingFeatures: Use cases that are supported with optimal tiling
        // bufferFeatures : Use cases that are supported for buffer
        vkGetPhysicalDeviceFormatProperties(vkSetup->_physicalDevice,format,&props);

        if (tiling==VK_IMAGE_TILING_LINEAR&&(props.linearTilingFeatures&features)==features)
        {
            return format;
        }else if (tiling==VK_IMAGE_TILING_OPTIMAL&&(props.optimalTilingFeatures&features)==features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}