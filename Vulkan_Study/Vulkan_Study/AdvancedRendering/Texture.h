#pragma once
#include "Image.h"
#include "VulkanSetup.h"

class Texture
{
public:
    //-Initialization and cleanup----------------------------------------------------
    void createTexture(VulkanSetup* pVkSetup, const VkCommandPool& commandPool,const Image& image);
    void cleanupTexture();
private:
    //-Texture sampler creation-------------------------------------------------------
    void createTextureSampler();
public:
    //-Members-------------------------------------------------------------------------
    VulkanSetup* _vkSetup;
    VulkanImage _textureImage;
    VkImageView _textureImageView;
    VkSampler _textureSampler;
};
