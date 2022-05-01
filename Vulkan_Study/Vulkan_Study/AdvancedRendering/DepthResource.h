#pragma once
#include "Image.h"
#include "VulkanSetup.h"

class DepthResource
{
public:
    //-Initialisation and cleanup----------------------------------------------------
    void createDepthResource(VulkanSetup* vkSetup, const VkExtent2D& extent, VkCommandPool commandPool);
    void cleanDepthResource();

    //Depth resource creation helpers------------------------------------------------------
    static VkFormat findDepthFormat(const VulkanSetup* vkSetup);
    static VkFormat findSupportedFormat(const VulkanSetup* vkSetup, const std::vector<VkFormat>& candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features);

    
public:
    //-Members---------------------------------------------------------------------
    VulkanSetup* _vkSetup;
    VulkanImage _depthImage;
    VkImageView _depthImageView;
};
