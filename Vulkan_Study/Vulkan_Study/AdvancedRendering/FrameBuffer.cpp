#include "FrameBuffer.h"

void BackFrameBuffer::initFramebuffer(VulkanSetup* vkSetup, const SwapChain* swapChain,const VkCommandPool& commandPool)
{
    _vkSetup=vkSetup;
    _depthResource.createDepthResource(_vkSetup,swapChain->_extent,commandPool);
    createFrameBuffers(swapChain);
}

void BackFrameBuffer::cleanupFramebuffers()
{
    _depthResource.cleanDepthResource();

    for (size_t i=0;i<_framebuffers.size();i++)
    {
        vkDestroyFramebuffer(_vkSetup->_device,_framebuffers[i],nullptr);
    }
}

void BackFrameBuffer::createFrameBuffers(const SwapChain* swapChain)
{
    _framebuffers.resize(swapChain->_imageViews.size());
    
}
