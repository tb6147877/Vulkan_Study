#pragma once
#include <cstdint>
#include <string>
#include <glm/ext/scalar_uint_sized.hpp>
#include <vulkan/vulkan_core.h>

#include "Buffer.h"


//A wrapper for a VkImage and its associated view and memory. along with some helper image utility static functions

//POD struct for an image from file(.png...)
struct Image
{
    uint32_t width;
    uint32_t height;
    VkFormat format;
    Buffer imageData;
};

class VulkanImage
{
public:
    //-Texture operation info structs----------------------------------------------------------
    struct ImageCreateInfo
    {
        uint32_t width=0;
        uint32_t height=0;
        VkFormat format=VK_FORMAT_UNDEFINED;
        VkImageTiling tiling=VK_IMAGE_TILING_OPTIMAL;//Texels are laid out in an implementation defined order for optimal access
        VkImageUsageFlags usage=0;
        uint32_t arrayLayers=1;//default to 1 for convenience
        VkMemoryPropertyFlags properties =0;
        VkImageCreateFlags flags=0;
        VulkanImage* pVulkanImage=nullptr;
    };

    struct LayoutTransitionInfo
    {
        VulkanImage* pVulkanImage=nullptr;
        VkCommandPool renderCommandPool=VK_NULL_HANDLE;
        VkFormat format=VK_FORMAT_UNDEFINED;
        VkImageLayout oldLayout=VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout newLayout=VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t arrayLayers=1;
    };

    //-Querying a format's support--------------------------------------------------------
    struct ImageFormatSupportDetails
    {
        VkFormat format;
        VkImageFormatProperties properties;
    };

public:
    //-Initialisation ans cleanup------------------------------------------------------
    static void createImage(const VulkanSetup* vkSetup, const VkCommandPool& commandPool, const ImageCreateInfo& info);
    void cleanupImage(const VulkanSetup* vkSetup);

    //-Image view creation---------------------------------------------------------------
    static VkImageView createImageView(const VulkanSetup* vkSetup, const VkImageViewCreateInfo& imageViewCreateInfo);

    //-Image layout transition---------------------------------------------------------------
    static void transitionImageLayout(const VulkanSetup* vkSetup,const LayoutTransitionInfo& transitionInfo);

    //-Image load from file----------------------------------------------------------------------
    static Image loadImageFromFile(const std::string& path);

    //-Helpers for image formats------------------------------------------------------------------
    static VkFormat getImageFormat(int numChannels);
    static ImageFormatSupportDetails queryFormatSupport(VkPhysicalDevice device, VkFormat format, VkImageType type,
        VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags);
    static VkBool32 formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format,VkImageTiling tiling);
    
public:
    VkExtent2D _extent={0,0};
    VkFormat _format=VK_FORMAT_UNDEFINED;
    VkImage _image=nullptr;
    VkDeviceMemory _imageMemory=nullptr;
};
