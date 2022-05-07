#include "DeferredRendering.h"

#include <set>
#include <stdexcept>

#include "Utils.h"

void DeferredRendering::initRenderer(VulkanSetup* pVkSetup, SwapChain* swapchain, Model* model){
    _vkSetup=pVkSetup;
    _swapChain=swapchain;
    _model=model;

    createCommandPool();
    createDescriptorSetLayout();
    createOutputRenderPass();
    createDeferredRenderPass();
    _backFrameBuffer.initFramebuffer(_vkSetup,_swapChain,_renderCommandPool,_outputRenderPass);
    createDeferredFramebuffer();
    createColorAttachmentSampler();
}
void DeferredRendering::cleanupRenderer(){
    //todo:clean all vulkan objs
}

void DeferredRendering::createFrameBufferAttachment(const std::string& name,VkFormat format,VkImageUsageFlagBits usage, const VkCommandPool& commandPool)
{
    FrameBufferAttachment* attachment=&_attachments[name];//[] inserts an element if non exist im map
    attachment->format=format;

    //create the image construction
    VulkanImage::ImageCreateInfo info{};
    info.width=_swapChain->_extent.width;
    info.height=_swapChain->_extent.height;
    info.format=attachment->format;
    info.tiling=VK_IMAGE_TILING_OPTIMAL;
    info.usage=usage|VK_IMAGE_USAGE_SAMPLED_BIT;
    info.pVulkanImage=&attachment->vulkanImage;

    VulkanImage::createImage(_vkSetup,commandPool,info);

    //create image view
    VkImageAspectFlags aspectMask = 0;
    //usage determines aspect mask
    if (usage&VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (usage&VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (aspectMask<=0)
    {
        throw std::runtime_error("Invalid aspect mask!");
    }

    VkImageViewCreateInfo imageViewCreateInfo=utils::initImageViewCreateInfo(attachment->vulkanImage._image,
        VK_IMAGE_VIEW_TYPE_2D,format,{},{aspectMask,0,1,0,1});
    attachment->imageView=VulkanImage::createImageView(_vkSetup,imageViewCreateInfo);
}

void DeferredRendering::createDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings={
        //binding 0: vertex shader uniform buffer
        utils::initDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
        //binding 1: position texture
        //A combined image sampler (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) is a single descriptor type
        //associated with both a sampler and an image resource, combining both a sampler and sampled
        //image descriptor into a single descriptor.
        utils::initDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 2: normal texture
        utils::initDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 3: albedo texture
        utils::initDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 4: fragment shader uniform buffer
        utils::initDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 5: fragment shader shadow map
        utils::initDescriptorSetLayoutBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    utils::createDescriptorSetLayout(&_vkSetup->_device,&_descriptorSetLayout,setLayoutBindings);
}

void DeferredRendering::createDeferredRenderPass()
{
    createFrameBufferAttachment("position",VK_FORMAT_R16G16B16A16_SFLOAT,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,_renderCommandPool);
    createFrameBufferAttachment("normal",VK_FORMAT_R16G16B16A16_SFLOAT,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,_renderCommandPool);
    createFrameBufferAttachment("albedo",VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,_renderCommandPool);
    createFrameBufferAttachment("depth",DepthResource::findDepthFormat(_vkSetup),VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,_renderCommandPool);
    
    std::array<VkAttachmentDescription, 4> attachmentDescriptions={};

    for (size_t i=0;i<4;i++)
    {
        attachmentDescriptions[i].samples=VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions[i].loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescriptions[i].stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[i].stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[i].initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
        if (i==3)
        {
            attachmentDescriptions[i].finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }else
        {
            attachmentDescriptions[i].finalLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }

    attachmentDescriptions[0].format=_attachments["position"].format;
    attachmentDescriptions[1].format=_attachments["normal"].format;
    attachmentDescriptions[2].format=_attachments["albedo"].format;
    attachmentDescriptions[3].format=_attachments["depth"].format;

    std::vector<VkAttachmentReference> colorReferences;
    colorReferences.push_back({0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    colorReferences.push_back({1,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    colorReferences.push_back({2,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    VkAttachmentReference depthReferences={3,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    
    VkSubpassDescription subpass{};//create subpass
    subpass.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments=colorReferences.data();
    subpass.colorAttachmentCount=static_cast<uint32_t>(colorReferences.size());
    subpass.pDepthStencilAttachment=&depthReferences;

    std::array<VkSubpassDependency,2> dependencies{};//dependencies for attachment layout transition
    dependencies[0].srcSubpass=VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass=0;
    dependencies[0].srcStageMask=VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask=VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags=VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass=0;
    dependencies[1].dstSubpass=VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask=VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask=VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask=VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags=VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderpassInfo{};
    renderpassInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassInfo.pAttachments=attachmentDescriptions.data();
    renderpassInfo.attachmentCount=static_cast<uint32_t>(attachmentDescriptions.size());
    renderpassInfo.subpassCount=1;
    renderpassInfo.pSubpasses=&subpass;
    renderpassInfo.dependencyCount=static_cast<uint32_t>(dependencies.size());
    renderpassInfo.pDependencies=dependencies.data();

    if (vkCreateRenderPass(_vkSetup->_device,&renderpassInfo,nullptr,&_deferredRenderPass)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create deferred render pass!");
    }
}

void DeferredRendering::createDeferredFramebuffer()
{
    std::array<VkImageView,4> attachmentViews;
    attachmentViews[0]=_attachments["position"].imageView;
    attachmentViews[1]=_attachments["normal"].imageView;
    attachmentViews[2]=_attachments["albedo"].imageView;
    attachmentViews[3]=_attachments["depth"].imageView;

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext=nullptr;
    createInfo.renderPass=_deferredRenderPass;
    createInfo.pAttachments=attachmentViews.data();
    createInfo.attachmentCount=static_cast<uint32_t>(attachmentViews.size());
    createInfo.width=_swapChain->_extent.width;
    createInfo.height=_swapChain->_extent.height;
    createInfo.layers=1;
    if (vkCreateFramebuffer(_vkSetup->_device,&createInfo,nullptr,&_deferredFrameBuffer)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create deferred frame buffer ");
    }
}

void DeferredRendering::createColorAttachmentSampler()
{
    //create sampler to sample from the color attachments
    VkSamplerCreateInfo sampler{};
    sampler.sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler.magFilter=VK_FILTER_NEAREST;
    sampler.minFilter=VK_FILTER_NEAREST;
    sampler.mipmapMode=VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeV=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeW=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.mipLodBias=0.0f;
    sampler.maxAnisotropy=1.0f;
    sampler.minLod=0.0f;
    sampler.maxLod=1.0f;
    sampler.borderColor=VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    if (vkCreateSampler(_vkSetup->_device,&sampler,nullptr,&_colorAttachmentSampler)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create color attachment sampler! ");
    }
}

void DeferredRendering::createPipeline()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo=utils::initPipelineLayoutCreateInfo(1,&_descriptorSetLayout);

    if (vkCreatePipelineLayout(_vkSetup->_device,&pipelineLayoutInfo,nullptr,&_pipelineLayout)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment=utils::initPipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT,VK_FALSE);
    
    VkViewport viewport{0.0f,0.0f,(float)_swapChain->_extent.width,(float)_swapChain->_extent.height,0.0f,1.0f};
    VkRect2D scissor{{0,0},_swapChain->_extent};

    VkShaderModule vertShaderModule, fragShaderModule;
    std::array<VkPipelineShaderStageCreateInfo,2> shaderStages;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly=utils::initPipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizer=utils::initPipelineRasterStateCreateInfo(VK_POLYGON_MODE_FILL,VK_CULL_MODE_FRONT_BIT,VK_FRONT_FACE_COUNTER_CLOCKWISE);
    VkPipelineColorBlendStateCreateInfo colorBlending=utils::initPipelineColorBlendStateCreateInfo(1,&colorBlendAttachment);
    VkPipelineDepthStencilStateCreateInfo depthStencil=utils::initPipelineDepthStencilStateCreateInfo(VK_TRUE,VK_TRUE,VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineViewportStateCreateInfo viewportState=utils::initPipelineViewportStateCreateInfo(1,&viewport,1,&scissor);
    VkPipelineMultisampleStateCreateInfo multisampling=utils::initPipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

    VkGraphicsPipelineCreateInfo pipelineInfo=utils::initGraphicsPipelineCreateInfo(_pipelineLayout,_outputRenderPass);

    pipelineInfo.stageCount=static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages=shaderStages.data();
    pipelineInfo.pInputAssemblyState=&inputAssembly;
    pipelineInfo.pViewportState=&viewportState;
    pipelineInfo.pRasterizationState=&rasterizer;
    pipelineInfo.pMultisampleState=&multisampling;
    pipelineInfo.pDepthStencilState=&depthStencil;
    pipelineInfo.pColorBlendState=&colorBlending;
    
    
}

void DeferredRendering::createUniformBuffers(){

}

void DeferredRendering::createDescriptorPool(){

}

void DeferredRendering::createDescriptorSets(){

}

void DeferredRendering::updateUniformBuffers(uint32_t curImage){

}

void DeferredRendering::createCommandBuffer(){

}

void DeferredRendering::createSyncObjects(){

}


void DeferredRendering::drawFrame()
{

    
}