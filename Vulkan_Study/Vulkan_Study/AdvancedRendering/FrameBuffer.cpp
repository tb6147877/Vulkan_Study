#include "FrameBuffer.h"

#include <filesystem>
#include <array>

void BackFrameBuffer::initFramebuffer(VulkanSetup* vkSetup, const SwapChain* swapChain,const VkCommandPool& commandPool,const VkRenderPass& renderpass)
{
    _vkSetup=vkSetup;
    _depthResource.createDepthResource(_vkSetup,swapChain->_extent,commandPool);
    createFrameBuffers(swapChain,renderpass);
}

void BackFrameBuffer::cleanupFramebuffers()
{
    _depthResource.cleanDepthResource();

    for (size_t i=0;i<_framebuffers.size();i++)
    {
        vkDestroyFramebuffer(_vkSetup->_device,_framebuffers[i],nullptr);
    }
}

void BackFrameBuffer::createFrameBuffers(const SwapChain* swapChain,const VkRenderPass& renderpass)
{
    //resize the container to hold all the framebuffers, or image views, in the swap chain
    _framebuffers.resize(swapChain->_imageViews.size());

    //now loop over the image views and create the framebuffers, also bind the image to the attachment
    for (size_t i=0;i<swapChain->_imageViews.size();i++)
    {
        std::array<VkImageView,2> attachments={swapChain->_imageViews[i], _depthResource._depthImageView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass=renderpass;//which renderpass the framebuffer needs, only one at the moment
        framebufferInfo.attachmentCount=static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments=attachments.data();
        framebufferInfo.width=swapChain->_extent.width;
        framebufferInfo.height=swapChain->_extent.height;
        framebufferInfo.layers=1;

        if (vkCreateFramebuffer(_vkSetup->_device,&framebufferInfo,nullptr,&_framebuffers[i])!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}
