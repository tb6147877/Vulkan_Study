#pragma once
#include "DepthResource.h"
#include "SwapChain.h"
#include "VulkanSetup.h"

//A class that contains the data related to the back frame buffer.


class BackFrameBuffer
{
public:
    //-Initialisation and cleanup-------------------------------------------
    void initFramebuffer(VulkanSetup* vkSetup, const SwapChain* swapChain,const VkCommandPool& commandPool);
    void cleanupFramebuffers();
    
private:
    //-Framebuffer creation--------------------------------------------------
    void createFrameBuffers(const SwapChain* swapChain);
    
public:
    //-Members-----------------------------------------------------------------
    VulkanSetup* _vkSetup;
    std::vector<VkFramebuffer> _framebuffers;
    DepthResource _depthResource;
};
