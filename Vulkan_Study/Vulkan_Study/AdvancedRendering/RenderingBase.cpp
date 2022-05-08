#include "RenderingBase.h"

#include <stdexcept>

#include "DepthResource.h"
#include "Utils.h"

void RenderingBase::createCommandPool()
{
    utils::createCommandPool(*_vkSetup,&_renderCommandPool,VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

void RenderingBase::createOutputRenderPass()
{
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
    colorAttachment.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//if this image is go to swap chain, it must be this option

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

    /**************************************************************************************************************
    // A single render pass consists of multiple subpasses, which are subsequent rendering operations depending on 
    // content of framebuffers on previous passes (eg post processing). Grouping subpasses into a single render 
    // pass lets Vulkan optimise every subpass references 1 or more attachments.
    **************************************************************************************************************/
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment=0;
    colorAttachmentRef.layout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment=1;
    depthAttachmentRef.layout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //the subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount=1;
    subpass.pColorAttachments=&colorAttachmentRef;
    subpass.pDepthStencilAttachment=&depthAttachmentRef;

    /**************************************************************************************************************
    // subpass dependencies control the image layout transitions. They specify memory and execution of dependencies
    // between subpasses there are implicit subpasses right before and after the render pass
    // There are two built-in dependencies that take care of the transition at the start of the render pass and at 
    // the end, but the former does not occur at the right time as it assumes that the transition occurs at the 
    // start of the pipeline, but we haven't acquired the image yet there are two ways to deal with the problem:
    // - change waitStages of the imageAvailableSemaphore (in drawframe) to VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
    // this ensures that the render pass does not start until image is available.
    // - make the render pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage.
    **************************************************************************************************************/
    VkSubpassDependency dependency{};
    dependency.srcSubpass=VK_SUBPASS_EXTERNAL;//the implicit subpass before or after the render pass depending on whether it is specified in srcSubpass or dstSubpass
    dependency.dstSubpass=0;//our subpass, The dstSubpass must always be higher than srcSubpass to prevent cycles in the dependency graph (unless one of the subpasses is VK_SUBPASS_EXTERNAL
    dependency.srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//wait for the swap chain to finish reading from the image before we can access it
    dependency.srcAccessMask=0;
    dependency.dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    /**************************************************************************************************************
    // Specify the operations to wait on and stages when ops occur.
    // Need to wait for swap chain to finish reading, can be accomplished by waiting on the colour attachment 
    // output stage.
    // Need to make sure there are no conflicts between transitionning og the depth image and it being cleared as 
    // part of its load operation.
    // The depth image is first accessed in the early fragment test pipeline stage and because we have a load 
    // operation that clears, we should specify the access mask for writes.
    **************************************************************************************************************/

    std::array<VkAttachmentDescription,2> attachments={colorAttachment,depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount=static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments=attachments.data();
    renderPassInfo.subpassCount=1;
    renderPassInfo.pSubpasses=&subpass;
    renderPassInfo.dependencyCount=1;// static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies=&dependency;//dependencies.data();

    if (vkCreateRenderPass(_vkSetup->_device,&renderPassInfo,nullptr,&_outputRenderPass)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderingBase::createUniformBuffers()
{
    VulkanBuffer::createUniformBuffer<UniformBufferObjectVert>(_vkSetup,MAX_FRAMES_IN_FLIGHT,&_vertUniformBuffer,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VulkanBuffer::createUniformBuffer<UniformBufferObjectFrag>(_vkSetup,MAX_FRAMES_IN_FLIGHT,&_fragUniformBuffer,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void RenderingBase::createDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[]={
        { VK_DESCRIPTOR_TYPE_SAMPLER,                DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DESCRIPTOR_POOL_NUM },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       DESCRIPTOR_POOL_NUM }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags=VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;// determines if individual descriptor sets can be freed or not
    poolInfo.maxSets=DESCRIPTOR_POOL_NUM*MAX_FRAMES_IN_FLIGHT;
    poolInfo.poolSizeCount=static_cast<uint32_t>(sizeof(poolSizes)/sizeof(VkDescriptorPoolSize));
    poolInfo.pPoolSizes=poolSizes;

    if (vkCreateDescriptorPool(_vkSetup->_device,&poolInfo,nullptr,&_descriptorPool)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void RenderingBase::updateVertUniformBuffer(uint32_t imgIndex, const UniformBufferObjectVert& ubo)
{
    void* data;
    vkMapMemory(_vkSetup->_device,_vertUniformBuffer._memory,sizeof(ubo)*imgIndex,sizeof(ubo),0,&data);
    memcpy(data,&ubo,sizeof(ubo));
    vkUnmapMemory(_vkSetup->_device,_vertUniformBuffer._memory);
}
void RenderingBase::updateFragUniformBuffer(uint32_t imgIndex, const UniformBufferObjectFrag& ubo)
{
    void* data;
    vkMapMemory(_vkSetup->_device,_fragUniformBuffer._memory,sizeof(ubo)*imgIndex,sizeof(ubo),0,&data);
    memcpy(data,&ubo,sizeof(ubo));
    vkUnmapMemory(_vkSetup->_device,_fragUniformBuffer._memory);
}
