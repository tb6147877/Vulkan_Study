#include "Texture.h"

#include <complex>
#include <stdexcept>

#include "Utils.h"

void Texture::createTexture(VulkanSetup* pVkSetup, const VkCommandPool& commandPool,const Image& image)
{
    _vkSetup=pVkSetup;
    VulkanBuffer stagingBuffer;//staging buffer containing image in host memory
    VulkanBuffer::CreateInfo createInfo{};
    createInfo.size=image.imageData.size;
    createInfo.usage=VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.properties=VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    createInfo.pVulkanBuffer=&stagingBuffer;

    VulkanBuffer::createBuffer(_vkSetup,&createInfo,VK_SHARING_MODE_EXCLUSIVE);

    void* data;
    vkMapMemory(_vkSetup->_device,stagingBuffer._memory,0,image.imageData.size,0,&data);
    memcpy(data,image.imageData.data,image.imageData.size);
    vkUnmapMemory(_vkSetup->_device,stagingBuffer._memory);

    //create the image and its memory
    VulkanImage::ImageCreateInfo imgCreateInfo{};
    imgCreateInfo.width=image.width;
    imgCreateInfo.height=image.height;
    imgCreateInfo.format=image.format;
    imgCreateInfo.tiling=VK_IMAGE_TILING_OPTIMAL;
    imgCreateInfo.usage=VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    imgCreateInfo.properties=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    imgCreateInfo.pVulkanImage=&_textureImage;

    VulkanImage::createImage(_vkSetup,commandPool,imgCreateInfo);

    //copy host data to device
    VulkanImage::LayoutTransitionInfo transitionData{};
    transitionData.pVulkanImage=&_textureImage;
    transitionData.renderCommandPool=commandPool;
    transitionData.format=image.format;
    transitionData.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    transitionData.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    VulkanImage::transitionImageLayout(_vkSetup,transitionData);//specify the initial layout VK_IMAGE_LAYOUT_UNDEFINED

    //need to specify which parts of the buffer we are going to copy to which part of the image
    std::vector<VkBufferImageCopy> regions={
        {
            0,0,0,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},
            {0,0,0},
            {image.width,image.height,1}
        }
    };
    
    VulkanBuffer::copyBufferToImage(_vkSetup,commandPool,stagingBuffer._buffer,_textureImage._image,regions);

    //need another transfer to give the shader access to the texture
    transitionData.pVulkanImage=&_textureImage;
    transitionData.renderCommandPool=commandPool;
    transitionData.format=image.format;
    transitionData.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transitionData.newLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VulkanImage::transitionImageLayout(_vkSetup,transitionData);

    stagingBuffer.cleanupBufferData(_vkSetup->_device);

    //create the image view
    VkImageViewCreateInfo imageViewCreateInfo=utils::initImageViewCreateInfo(_textureImage._image,
        VK_IMAGE_VIEW_TYPE_2D,image.format,{},{VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1});
    _textureImageView=VulkanImage::createImageView(_vkSetup,imageViewCreateInfo);
    

    //create the sampler
    createTextureSampler();
}

void Texture::cleanupTexture()
{
    vkDestroySampler(_vkSetup->_device,_textureSampler,nullptr);
    vkDestroyImageView(_vkSetup->_device,_textureImageView,nullptr);
    _textureImage.cleanupImage(_vkSetup);
}


void Texture::createTextureSampler()
{
    //configure the sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    //how to interpolate texels that are magnified or minified
    samplerInfo.magFilter=VK_FILTER_LINEAR;
    samplerInfo.minFilter=VK_FILTER_LINEAR;
    //addressing mode
    samplerInfo.addressModeU=VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV=VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW=VK_SAMPLER_ADDRESS_MODE_REPEAT;
    // VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions.
    // VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts the coordinates to mirror the image when going beyond the dimensions.
    // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : Take the color of the edge closest to the coordinate beyond the image dimensions.
    // VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE : Like clamp to edge, but instead uses the edge opposite to the closest edge.
    // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER : Return a solid color when sampling beyond the dimensions of the image

    //use unless performance is a concern
    samplerInfo.anisotropyEnable=VK_TRUE;

    //limits texel samples that used to calculate final colors
    samplerInfo.maxAnisotropy=_vkSetup->_deviceProperties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor=VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    //which coordinate system we want to use to address texels!
    samplerInfo.unnormalizedCoordinates=VK_FALSE;

    //if comparison enabled, texels will be compared to a value and result is used in filtering (useful for shadow maps)
    samplerInfo.compareEnable=VK_FALSE;
    samplerInfo.compareOp=VK_COMPARE_OP_ALWAYS;

    //mipmapping fields
    samplerInfo.mipmapMode=VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias=0.0f;
    samplerInfo.minLod=0.0f;
    samplerInfo.maxLod=0.0f;

    //now create the configured sampler
    if (vkCreateSampler(_vkSetup->_device,&samplerInfo,nullptr,&_textureSampler)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}