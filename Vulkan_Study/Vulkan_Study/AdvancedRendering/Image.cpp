#include "Image.h"

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

#include "Utils.h"

void VulkanImage::createImage(const VulkanSetup* vkSetup, const VkCommandPool& commandPool, const ImageCreateInfo& info)
{
    //create image ready to accept data on device
    VkImageCreateInfo imageInfo{};
    imageInfo.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType=VK_IMAGE_TYPE_2D;//coordiante system of the texels
    imageInfo.extent.width=info.width;//the dimensions of the image
    imageInfo.extent.height=info.height;
    imageInfo.extent.depth=1;
    imageInfo.mipLevels=1;//mip mapping
    imageInfo.arrayLayers=info.arrayLayers;
    imageInfo.format=info.format;//same format as the pixels is best
    imageInfo.tiling=info.tiling;//tiling of the pixels, ley vulkan lay them out
    imageInfo.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage=info.usage;//same semantics as during buffer creation, here as destination for the buffer copy
    imageInfo.samples=VK_SAMPLE_COUNT_1_BIT;//for multisampling
    imageInfo.sharingMode=VK_SHARING_MODE_EXCLUSIVE;//used by a single queue family
    imageInfo.flags=info.flags;// optional, for sparse data or cube maps

    //create the image.The hardware could fail for the format we have specified
    //We should have a list of acceptable formats and choose the best one depending on the selection of formats supported by the device
    if (vkCreateImage(vkSetup->_device,&imageInfo,nullptr,&info.pVulkanImage->_image)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkSetup->_device,info.pVulkanImage->_image,&memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize=memRequirements.size;
    allocInfo.memoryTypeIndex=utils::findMemoryType(&vkSetup->_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(vkSetup->_device,&allocInfo,nullptr,&info.pVulkanImage->_imageMemory)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(vkSetup->_device,info.pVulkanImage->_image,info.pVulkanImage->_imageMemory,0);

    //update the image's format
    info.pVulkanImage->_format=info.format;
}

void VulkanImage::cleanupImage(const VulkanSetup* vkSetup)
{
    //destroy the texture image and its memory
    vkDestroyImage(vkSetup->_device,_image,nullptr);
    vkFreeMemory(vkSetup->_device,_imageMemory,nullptr);
}

VkImageView VulkanImage::createImageView(const VulkanSetup* vkSetup, const VkImageViewCreateInfo& imageViewCreateInfo)
{
    VkImageView imageView;
    if (vkCreateImageView(vkSetup->_device, &imageViewCreateInfo,nullptr,&imageView)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image view!");
    }
    return imageView;
}

void VulkanImage::transitionImageLayout(const VulkanSetup* vkSetup,const LayoutTransitionInfo& transitionInfo)
{
    //image may have different layout, specify which layout we are transitioning to and from for optimal layout
    VkCommandBuffer commandBuffer=utils::beginSingleTimeCommands(&vkSetup->_device,transitionInfo.renderCommandPool);

    //perform layout transition using a image memory barrier, generally for syncing access to a resource
    //eg: make sure writes completes before subsequent read, but can transition image layout and transfer queue family ownership
    VkImageMemoryBarrier barrier{};
    barrier.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //specify layout transition
    barrier.oldLayout=transitionInfo.oldLayout;
    barrier.newLayout=transitionInfo.newLayout;
    //indices of the two queue family
    barrier.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;//todo: i don't know why
    barrier.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
    barrier.image=transitionInfo.pVulkanImage->_image;

    //if for depth view should only contain depth aspect
    if (transitionInfo.newLayout==VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;
        //include stencil aspect if available
        if (utils::hasStencilComponent(transitionInfo.format))
        {
            barrier.subresourceRange.aspectMask|=VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        
    }else
    {
        //color aspect of the image
        //The subresourceRange field describes what the image’s purpose is and which part of the image should be accessed.
        barrier.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
    }

    //mip mapping
    barrier.subresourceRange.baseMipLevel=0;
    barrier.subresourceRange.levelCount=1;
    //image array
    barrier.subresourceRange.baseArrayLayer=0;
    barrier.subresourceRange.layerCount=transitionInfo.arrayLayers;

    //we need to set the access mask and pipeline stage based on the layout in the transition
    //we need to handle two transitions
    //1 undefined->transfer dst: transfer writs that don't need to wait on anything
    //2 transfer dst->shader reading: share read should wait on transfer write
    
    //declare stages
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    //determine which of the two transitions we are executing:
    //first transition: src layout is not important, to a optimal layout as destination
    if (transitionInfo.oldLayout==VK_IMAGE_LAYOUT_UNDEFINED&&transitionInfo.newLayout==VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask=0;
        barrier.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
        // for the pre-barrier operations
        sourceStage=VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        // transfer writes must occur in the pipeline transfer stage, a pseudo-stage where transfers happen
        destinationStage=VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    //second transition: src layout is optimal, to layout optimal for sampling by a shader
    else if (transitionInfo.oldLayout==VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL&&transitionInfo.newLayout==VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;//wait on the transfer to finish writing
        barrier.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;

        sourceStage=VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage=VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    // third transition: src layout is not important, dst layout optimal for depth/stencil operations
    else if (transitionInfo.oldLayout==VK_IMAGE_LAYOUT_UNDEFINED&&transitionInfo.newLayout==VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask=0;
        barrier.dstAccessMask=VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage=VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage=VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    //extend this function for other transitions
    else
    {
        //unrecognised transition
        throw std::runtime_error("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer,sourceStage,destinationStage,0,
        //arrays of pipeline barriers of three available types 
        0,nullptr, //memory barrier
        0,nullptr,  //buffer memory barrier
        1,&barrier);//image memory barrier

    utils::endSingleTimeCommands(&vkSetup->_device,&vkSetup->_graphicsQueue,&commandBuffer,&transitionInfo.renderCommandPool);
}

Image VulkanImage::loadImageFromFile(const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);
    Image image{};
    int width, height, channels;
    unsigned char* data=stbi_load(path.c_str(),&width,&height,&channels,0);
    if (!data)
    {
        throw std::runtime_error("could not load image");
    }

    image.imageData.data=(unsigned char*)malloc(width*height*channels*sizeof(unsigned char));
    memcpy(image.imageData.data,data,width*height*channels);
    stbi_image_free(data);

    image.width=width;
    image.height=height;
    image.imageData.size=image.width*image.height*channels*sizeof(unsigned char);
    image.format=getImageFormat(channels);

    //Pixels need to be freed! do so when no longer in use at the end of application lifecycle
    return image;
}

VkFormat VulkanImage::getImageFormat(int numChannels)
{
    switch (numChannels)
    {
    case 1:
        return VK_FORMAT_R8_SRGB;
    case 2:
        return VK_FORMAT_R8G8_SRGB;
    case 3:
        return VK_FORMAT_R8G8B8_SRGB;
    case 4:
        return VK_FORMAT_R8G8B8A8_SRGB;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}

VulkanImage::ImageFormatSupportDetails VulkanImage::queryFormatSupport(VkPhysicalDevice device, VkFormat format, VkImageType type,
    VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags)
{
    //giving a set of desired image parameters, determine if a format is supported or not
    VulkanImage::ImageFormatSupportDetails details={format,{}};
    if (vkGetPhysicalDeviceImageFormatProperties(device,format,type,tiling,usage,flags,&details.properties)!=VK_SUCCESS)
    {
        throw std::runtime_error("format is not support!");
    }
    return details;
}


VkBool32 VulkanImage::formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format,VkImageTiling tiling)
{
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

    if (tiling==VK_IMAGE_TILING_OPTIMAL)
    {
        return formatProps.optimalTilingFeatures&VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    }

    if (tiling==VK_IMAGE_TILING_LINEAR)
    {
        return formatProps.linearTilingFeatures&VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    }

    return false;
}