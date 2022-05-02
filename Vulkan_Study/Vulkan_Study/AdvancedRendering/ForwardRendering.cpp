#include "ForwardRendering.h"
#include <array>
#include <stdexcept>

#include "AppConstants.h"
#include "DepthResource.h"
#include "Shader.h"
#include "Utils.h"


void ForwardRendering::initRenderer(VulkanSetup* pVkSetup,SwapChain* swapchain, Model* model)
{
    _vkSetup=pVkSetup;
    _swapChain=swapchain;
    _model=model;
    createDescriptorSetLayout();
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

    if (vkCreateRenderPass(_vkSetup->_device,&renderPassInfo,nullptr,&_renderPass)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
    
}


void ForwardRendering::createPipeline()
{
    VkShaderModule vertShaderModule = Shader::createShaderModule(_vkSetup,Shader::readFile(FWD_VERT_SHADER));
    VkShaderModule fragShaderModule = Shader::createShaderModule(_vkSetup,Shader::readFile(FWD_FRAG_SHADER));

    auto bindingDescription =_model->getBindingDescriptions(0);
    auto attributeDescriptions=_model->getAttributeDescriptions(0);

    VkViewport viewport{0.0f,0.0f,(float)_swapChain->_extent.width,(float)_swapChain->_extent.height,0.0f,1.0f};
    VkRect2D scissor{{0,0},_swapChain->_extent};

    VkPipelineColorBlendAttachmentState colorBlendAttachment=utils::initPipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT,VK_FALSE);

    
    vkDestroyShaderModule(_vkSetup->_device, vertShaderModule,nullptr);
    vkDestroyShaderModule(_vkSetup->_device, fragShaderModule,nullptr);
}

void ForwardRendering::createDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings={
        //binding 0: vertex shader uniform buffer
        utils::initDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
        //binding 1: diffuse texture
        //A combined image sampler (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) is a single descriptor type
        //associated with both a sampler and an image resource, combining both a sampler and sampled
        //image descriptor into a single descriptor.
        utils::initDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 2: fragment shader uniform buffer
        utils::initDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount=static_cast<uint32_t>(setLayoutBindings.size());
    layoutCreateInfo.pBindings=setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(_vkSetup->_device,&layoutCreateInfo,nullptr, &_descriptorSetLayout)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}