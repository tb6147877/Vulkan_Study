#include "ForwardRendering.h"

#include "DepthResource.h"


void ForwardRendering::initRenderer(VulkanSetup* pVkSetup,SwapChain* swapchain)
{
    _vkSetup=pVkSetup;
    _swapChain=swapchain;
    createRenderPass();
    createPipeline();
}

void ForwardRendering::cleanupRenderer()
{
    vkDestroyPipeline(_vkSetup->_device,_pipeline,nullptr);
    vkDestroyPipelineLayout(_vkSetup->_device,_pipelineLayout,nullptr);
    vkDestroyRenderPass(_vkSetup->_device,_renderPass,nullptr);
    
}

void ForwardRendering::createRenderPass(){
    //specify a color attachment to the render pass
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format=_swapChain->_imageFormat;//swapchian image format
    colorAttachment.samples=VK_SAMPLE_COUNT_1_BIT;//for multisample
    //The loadOp and storeOp determine what to do with the data in the attachment
    //before rendering and after rendering.
    colorAttachment.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;//Clear the values to a constant at the start
    colorAttachment.storeOp=VK_ATTACHMENT_STORE_OP_STORE;//Rendered contents will be stored in memory and can be read later
    colorAttachment.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //specify a depth attachment to the render pass
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format=DepthResource::findDepthFormat(_vkSetup);
    depthAttachment.samples=VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    
}


void ForwardRendering::createPipeline()
{
    
}