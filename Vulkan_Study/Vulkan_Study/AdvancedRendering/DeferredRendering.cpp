/*#include "DeferredRendering.h"

#include <set>
#include <stdexcept>

#include "AppConstants.h"
#include "Shader.h"
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
    createPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createCommandBuffer();
    createSyncObjects();
}
void DeferredRendering::cleanupRenderer(){
    RenderingBase::cleanupRenderer();
    vkDestroyRenderPass(_vkSetup->_device,_deferredRenderPass,nullptr);
    vkDestroyCommandPool(_vkSetup->_device,_renderCommandPool,nullptr);
    
    vkDestroyDescriptorSetLayout(_vkSetup->_device,_descriptorSetLayout,nullptr);
    _backFrameBuffer.cleanupFramebuffers();
    
    vkDestroyFramebuffer(_vkSetup->_device,_deferredFrameBuffer,nullptr);
    vkDestroySampler(_vkSetup->_device,_colorAttachmentSampler,nullptr);
    for(auto& kv:_attachments)
    {
        kv.second.vulkanImage.cleanupImage(_vkSetup);
        vkDestroyImageView(_vkSetup->_device,kv.second.imageView,nullptr);
    }
    
    vkDestroyPipelineLayout(_vkSetup->_device,_pipelineLayout,nullptr);
    vkDestroyPipeline(_vkSetup->_device,_combinePipeline,nullptr);
    vkDestroyPipeline(_vkSetup->_device,_gbufferPipeline,nullptr);

    for (size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
    {
        vkDestroySemaphore(_vkSetup->_device,_gbufferSemaphores[i],nullptr);
        vkDestroySemaphore(_vkSetup->_device,_imageAvailableSemaphores[i],nullptr);
        vkDestroySemaphore(_vkSetup->_device,_renderFinishedSemaphores[i],nullptr);
        vkDestroyFence(_vkSetup->_device,_inFlightFences[i],nullptr);
    }
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

    //refer to 1 color attachment
    VkColorComponentFlags colBlendAttachFlag=VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendAttachmentState colorBlendAttachment=utils::initPipelineColorBlendAttachmentState(colBlendAttachFlag,VK_FALSE);
    
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

    //combine pipeline
    vertShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(DFD_COMBINE_VERT_SHADER));
    fragShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(DFD_COMBINE_FRAG_SHADER));

    shaderStages[0]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,vertShaderModule,"main");
    shaderStages[1]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,fragShaderModule,"main");

    rasterizer.cullMode=VK_CULL_MODE_FRONT_BIT;

    VkPipelineVertexInputStateCreateInfo emptyInputStateInfo=utils::initPipelineVertexInputStateCreateInfo(0,nullptr,0,nullptr);
    pipelineInfo.pVertexInputState=&emptyInputStateInfo;

    if (vkCreateGraphicsPipelines(_vkSetup->_device,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&_combinePipeline)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create combine pipeline!");
    }

    vkDestroyShaderModule(_vkSetup->_device,vertShaderModule,nullptr);
    vkDestroyShaderModule(_vkSetup->_device,fragShaderModule,nullptr);

    //gbuffer pipeline
    pipelineInfo.renderPass=_deferredRenderPass;

    vertShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(DFD_GBUFFER_VERT_SHADER));
    fragShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(DFD_GBUFFER_FRAG_SHADER));

    shaderStages[0]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,vertShaderModule,"main");
    shaderStages[1]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,fragShaderModule,"main");

    rasterizer.cullMode=VK_CULL_MODE_BACK_BIT;

    auto bindingDescription = _model->getBindingDescriptions(0);
    auto attributeDescriptions = _model->getAttributeDescriptions(0);
    VkPipelineVertexInputStateCreateInfo vertexInputInfo=utils::initPipelineVertexInputStateCreateInfo(1,&bindingDescription,
        static_cast<uint32_t>(attributeDescriptions.size()),attributeDescriptions.data());

    pipelineInfo.pVertexInputState=&vertexInputInfo;

    //refer to 3 color attachment
    std::array<VkPipelineColorBlendAttachmentState,3> colorBlendAttachmentStates={
        utils::initPipelineColorBlendAttachmentState(colBlendAttachFlag,VK_FALSE),
        utils::initPipelineColorBlendAttachmentState(colBlendAttachFlag,VK_FALSE),
        utils::initPipelineColorBlendAttachmentState(colBlendAttachFlag,VK_FALSE)
    };

    colorBlending.attachmentCount=static_cast<uint32_t>(colorBlendAttachmentStates.size());
    colorBlending.pAttachments=colorBlendAttachmentStates.data();

    if (vkCreateGraphicsPipelines(_vkSetup->_device,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&_gbufferPipeline)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create combine pipeline!");
    }

    vkDestroyShaderModule(_vkSetup->_device,vertShaderModule,nullptr);
    vkDestroyShaderModule(_vkSetup->_device,fragShaderModule,nullptr);
}

void DeferredRendering::createDescriptorSets(){
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo=utils::initDescriptorSetAllocInfo(_descriptorPool,1,layouts.data());

    //gbuffer descriptor set
    if (vkAllocateDescriptorSets(_vkSetup->_device,&allocInfo,&_gbufferDescriptorSet)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to alloc gbuffer descriptor set!");
    }

    VkDescriptorBufferInfo gbufferUBOInfo{};
    gbufferUBOInfo.buffer=_vertUniformBuffer._buffer;
    gbufferUBOInfo.offset=0;
    gbufferUBOInfo.range=sizeof(UniformBufferObjectVert);
    if (!_vkSetup->isUniformBufferOffsetValid(gbufferUBOInfo.offset))
    {
        throw std::runtime_error("vertex uniform buffer do not fit data alignment!");
    }

    VkDescriptorImageInfo texDescriptor{};
    texDescriptor.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptor.imageView=_model->_textures[0]._textureImageView;
    texDescriptor.sampler=_model->_textures[0]._textureSampler;

    writeDescriptorSets={
        utils::initWriteDescriptorSet(_gbufferDescriptorSet,0,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,&gbufferUBOInfo),
        utils::initWriteDescriptorSet(_gbufferDescriptorSet,1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&texDescriptor)
    };

    vkUpdateDescriptorSets(_vkSetup->_device,static_cast<uint32_t>(writeDescriptorSets.size()),writeDescriptorSets.data(),0,nullptr);

    //combine descriptor sets
    allocInfo.descriptorSetCount=static_cast<uint32_t>(layouts.size());
    _descriptorSets.resize(layouts.size());

    if (vkAllocateDescriptorSets(_vkSetup->_device,&allocInfo,_descriptorSets.data())!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    

    VkDescriptorImageInfo texDescriptorPosition{};
    texDescriptorPosition.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptorPosition.imageView=_attachments["position"].imageView;
    texDescriptorPosition.sampler=_colorAttachmentSampler;

    VkDescriptorImageInfo texDescriptorNormal{};
    texDescriptorNormal.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptorNormal.imageView=_attachments["normal"].imageView;
    texDescriptorNormal.sampler=_colorAttachmentSampler;

    VkDescriptorImageInfo texDescriptorAlbedo{};
    texDescriptorAlbedo.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptorAlbedo.imageView=_attachments["albedo"].imageView;
    texDescriptorAlbedo.sampler=_colorAttachmentSampler;
    
    
    for (size_t i=0;i<_descriptorSets.size();i++)
    {
        VkDescriptorBufferInfo fragDescriptor{};
        fragDescriptor.buffer=_fragUniformBuffer._buffer;
        fragDescriptor.offset=sizeof(UniformBufferObjectFrag)*i;
        fragDescriptor.range=sizeof(UniformBufferObjectFrag);
        if (!_vkSetup->isUniformBufferOffsetValid(fragDescriptor.offset))
        {
            throw std::runtime_error("fragment uniform buffer do not fit data alignment!");
        }

        writeDescriptorSets={
            utils::initWriteDescriptorSet(_descriptorSets[i],1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&texDescriptorPosition),
            utils::initWriteDescriptorSet(_descriptorSets[i],2,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&texDescriptorNormal),
            utils::initWriteDescriptorSet(_descriptorSets[i],3,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&texDescriptorAlbedo),
            utils::initWriteDescriptorSet(_descriptorSets[i],4,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,&fragDescriptor)
        };

        vkUpdateDescriptorSets(_vkSetup->_device,static_cast<uint32_t>(writeDescriptorSets.size()),writeDescriptorSets.data(),0,nullptr);
    }
}

void DeferredRendering::updateUniformBuffers(uint32_t curImage){
    glm::mat4 proj=_camera->getProjectionMatrix(_swapChain->_extent.width/(float)_swapChain->_extent.height,0.1f,40.0f);
    proj[1][1]*=-1.0f;//y coordinates inverted, vulkan origin top left vs opengl bottom left
    glm::mat4 model=glm::mat4(1.0f);
   

    UniformBufferObjectVert vertUBO{};
    vertUBO.model=model;
    vertUBO.view=_camera->getViewMatrix();
    vertUBO.proj=proj;

    updateVertUniformBuffer(curImage,vertUBO);

    UniformBufferObjectFrag fragUBO{};
    fragUBO.viewPos=glm::vec4(_camera->getPosition().x,_camera->getPosition().y,_camera->getPosition().z,0.0);
    fragUBO.pointLights[0]={ {5.0f, 5.0f, 5.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 40.0f} };
    fragUBO.gap=glm::vec4(0.0);

    updateFragUniformBuffer(curImage,fragUBO);
}

void DeferredRendering::createCommandBuffer(){
    _gbufferCommandBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    _combineCommandBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    utils::createCommandBuffers(*_vkSetup,static_cast<uint32_t>(_gbufferCommandBuffer.size()),_gbufferCommandBuffer.data(),_renderCommandPool);
    utils::createCommandBuffers(*_vkSetup,static_cast<uint32_t>(_combineCommandBuffer.size()),_combineCommandBuffer.data(),_renderCommandPool);
}

void DeferredRendering::createSyncObjects(){
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _gbufferSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags=VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
    {
        if (vkCreateSemaphore(_vkSetup->_device,&semaphoreInfo,nullptr,&_imageAvailableSemaphores[i])!=VK_SUCCESS
            ||vkCreateSemaphore(_vkSetup->_device,&semaphoreInfo,nullptr,&_renderFinishedSemaphores[i])!=VK_SUCCESS
            ||vkCreateSemaphore(_vkSetup->_device,&semaphoreInfo,nullptr,&_gbufferSemaphores[i])!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }

        if (vkCreateFence(_vkSetup->_device,&fenceInfo,nullptr,&_inFlightFences[i])!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fences!");
        }
    }
}

void DeferredRendering::recordCommandBuffers()
{
    for (size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
    {
        recordGBufferCommandBuffer(_gbufferCommandBuffer[i]);
    }
}

void DeferredRendering::recordGBufferCommandBuffer(VkCommandBuffer cmdBuffer)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo=utils::initCommandBufferBeginInfo();

    //implicitly resets cmd buffer
    if (vkBeginCommandBuffer(cmdBuffer,&commandBufferBeginInfo)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    std::array<VkClearValue, 4> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[3].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass=_deferredRenderPass;
    renderPassBeginInfo.framebuffer=_deferredFrameBuffer;
    renderPassBeginInfo.renderArea.offset={0,0};
    renderPassBeginInfo.renderArea.extent=_swapChain->_extent;
    renderPassBeginInfo.clearValueCount=static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues=clearValues.data();

    //begin recording
    vkCmdBeginRenderPass(cmdBuffer,&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);

    //bind pipeline
    vkCmdBindPipeline(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_gbufferPipeline);

    //bind vertex buffer
    VkDeviceSize offset=0;
    vkCmdBindVertexBuffers(cmdBuffer,0,1,&_model->_vertexBuffer._buffer,&offset);

    //bind index buffer
    vkCmdBindIndexBuffer(cmdBuffer,_model->_indexBuffer._buffer,0,VK_INDEX_TYPE_UINT32);

    //bind descriptor
    vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_pipelineLayout,0,1,&_gbufferDescriptorSet,0,nullptr);

    //draw
    vkCmdDrawIndexed(cmdBuffer, _model->getIndicesNum(),1,0,0,0);
    
    //end recording
    vkCmdEndRenderPass(cmdBuffer);

    if (vkEndCommandBuffer(cmdBuffer)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to end recording command buffer!");
    }
}

void DeferredRendering::recordRenderCommandBuffer(VkCommandBuffer cmdBuffer,uint32_t imgIndex)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo=utils::initCommandBufferBeginInfo();

    //implicitly resets cmd buffer
    if (vkBeginCommandBuffer(cmdBuffer,&commandBufferBeginInfo)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    std::array<VkClearValue,2> clearValues{};
    clearValues[0].color={0.0f,0.0f,0.0f,1.0f};
    clearValues[1].depthStencil={1.0f,0};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass=_outputRenderPass;
    renderPassBeginInfo.framebuffer=_backFrameBuffer._framebuffers[imgIndex];
    renderPassBeginInfo.renderArea.offset={0,0};
    renderPassBeginInfo.renderArea.extent=_swapChain->_extent;
    renderPassBeginInfo.clearValueCount=static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues=clearValues.data();

    //begin recording
    vkCmdBeginRenderPass(cmdBuffer,&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);

    //bind pipeline
    vkCmdBindPipeline(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_combinePipeline);

    //bind descriptor
    vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_pipelineLayout,0,1,&_descriptorSets[_currentFrame],0,nullptr);

    //draw
    vkCmdDraw(cmdBuffer, 3,1,0,0);
    
    //end recording
    vkCmdEndRenderPass(cmdBuffer);

    if (vkEndCommandBuffer(cmdBuffer)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to end recording command buffer!");
    }
}


void DeferredRendering::drawFrame()
{
    vkWaitForFences(_vkSetup->_device,1,&_inFlightFences[_currentFrame],VK_TRUE,UINT64_MAX);

    VkResult result=vkAcquireNextImageKHR(_vkSetup->_device,_swapChain->_swapChain,UINT64_MAX,
        _imageAvailableSemaphores[_currentFrame],VK_NULL_HANDLE,&_scImageIndex);

    if (result==VK_ERROR_OUT_OF_DATE_KHR)
    {
        //todo: recreate swapchain
        return;
    }else if (result!=VK_SUCCESS&&result!=VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffers(_currentFrame);
    recordRenderCommandBuffer(_combineCommandBuffer[_currentFrame],_scImageIndex);
    
    VkPipelineStageFlags waitStage=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};

    //gbuffer rendering
    submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask=&waitStage;

    submitInfo.waitSemaphoreCount=1;
    submitInfo.pWaitSemaphores=&_imageAvailableSemaphores[_currentFrame];

    submitInfo.commandBufferCount=1;
    submitInfo.pCommandBuffers=&_gbufferCommandBuffer[_currentFrame];

    submitInfo.signalSemaphoreCount=1;
    submitInfo.pSignalSemaphores=&_gbufferSemaphores[_currentFrame];

    if (vkQueueSubmit(_vkSetup->_graphicsQueue,1,&submitInfo,VK_NULL_HANDLE)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw gbuffer command buffer!");
    }

    //combine rendering
    submitInfo.waitSemaphoreCount=1;
    submitInfo.pWaitSemaphores=&_gbufferSemaphores[_currentFrame];

    submitInfo.signalSemaphoreCount=1;
    submitInfo.pSignalSemaphores=&_renderFinishedSemaphores[_currentFrame];

    submitInfo.commandBufferCount=1;
    submitInfo.pCommandBuffers=&_combineCommandBuffer[_currentFrame];

    vkResetFences(_vkSetup->_device,1,&_inFlightFences[_currentFrame]);

    if (vkQueueSubmit(_vkSetup->_graphicsQueue,1,&submitInfo,_inFlightFences[_currentFrame])!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw combined command buffer!");
    }
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount=1;
    presentInfo.pWaitSemaphores=&_renderFinishedSemaphores[_currentFrame];

    presentInfo.swapchainCount=1;
    presentInfo.pSwapchains=&_swapChain->_swapChain;
    presentInfo.pImageIndices=&_scImageIndex;

    result=vkQueuePresentKHR(_vkSetup->_presentQueue,&presentInfo);

   
    if (result==VK_ERROR_OUT_OF_DATE_KHR||result==VK_SUBOPTIMAL_KHR||_frameBufferResized)
    {
        _frameBufferResized=false;
        //todo:recreate swap chain
    }else if (result!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
    

    _currentFrame=(_currentFrame+1)%MAX_FRAMES_IN_FLIGHT;
    
    
}*/

//===========================================================================================

#include "DeferredRendering.h"

#include <set>
#include <stdexcept>

#include "AppConstants.h"
#include "Shader.h"
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
    createPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createCommandBuffer();
    createSyncObjects();
    initAodDemo();
}

void DeferredRendering::cleanupRenderer(){
    RenderingBase::cleanupRenderer();
    vkDestroyRenderPass(_vkSetup->_device,_deferredRenderPass,nullptr);
    vkDestroyCommandPool(_vkSetup->_device,_renderCommandPool,nullptr);
    
    vkDestroyDescriptorSetLayout(_vkSetup->_device,_descriptorSetLayout,nullptr);
    _backFrameBuffer.cleanupFramebuffers();
    
    vkDestroyFramebuffer(_vkSetup->_device,_deferredFrameBuffer,nullptr);
    vkDestroySampler(_vkSetup->_device,_colorAttachmentSampler,nullptr);
    for(auto& kv:_attachments)
    {
        kv.second.vulkanImage.cleanupImage(_vkSetup);
        vkDestroyImageView(_vkSetup->_device,kv.second.imageView,nullptr);
    }
    
    vkDestroyPipelineLayout(_vkSetup->_device,_pipelineLayout,nullptr);
    vkDestroyPipeline(_vkSetup->_device,_combinePipeline,nullptr);
    vkDestroyPipeline(_vkSetup->_device,_gbufferPipeline,nullptr);
    vkDestroyPipeline(_vkSetup->_device,_finalPipeline,nullptr);

    for (size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
    {
        vkDestroySemaphore(_vkSetup->_device,_gbufferSemaphores[i],nullptr);
        vkDestroySemaphore(_vkSetup->_device,_imageAvailableSemaphores[i],nullptr);
        vkDestroySemaphore(_vkSetup->_device,_renderFinishedSemaphores[i],nullptr);
        vkDestroyFence(_vkSetup->_device,_inFlightFences[i],nullptr);
    }
}

void DeferredRendering::initAodDemo()
{
    _aodTextures.resize(AOD_PIC_PATHS.size());
    for (size_t i=0;i<AOD_PIC_PATHS.size();i++)
    {
        Image img=VulkanImage::loadImageFromFile(AOD_PIC_PATHS[i]);
        _aodTextures[i].createTexture(_vkSetup,_renderCommandPool,img);
    }
}

void DeferredRendering::createFrameBufferAttachment(const std::string& name,VkFormat format,VkImageUsageFlagBits usage, const VkCommandPool& commandPool,const int index)
{
    FrameBufferAttachment* attachment=&_attachments[name];//[] inserts an element if non exist im map
    attachment->format=format;

    //create the image construction
    VulkanImage::ImageCreateInfo info{};
    info.width=_swapChain->_extent.width;
    info.height=_swapChain->_extent.height;
    info.format=attachment->format;
    info.tiling=VK_IMAGE_TILING_OPTIMAL;
    if (index==0)
    {
        info.usage=usage|VK_IMAGE_USAGE_SAMPLED_BIT;
    }else
    {
        info.usage=usage|VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    
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
        utils::initDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 2: normal texture
        utils::initDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 3: albedo texture
        utils::initDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 4: fragment shader uniform buffer
        utils::initDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 5: output texture
        utils::initDescriptorSetLayoutBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 6:aod test
        utils::initDescriptorSetLayoutBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,9)
        
    };
    utils::createDescriptorSetLayout(&_vkSetup->_device,&_descriptorSetLayout,setLayoutBindings);
}

void DeferredRendering::createDeferredRenderPass()
{
    createFrameBufferAttachment("output",_swapChain->_imageFormat,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,_renderCommandPool,0);
    createFrameBufferAttachment("position",VK_FORMAT_R16G16B16A16_SFLOAT,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,_renderCommandPool,1);
    createFrameBufferAttachment("normal",VK_FORMAT_R16G16B16A16_SFLOAT,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,_renderCommandPool,2);
    createFrameBufferAttachment("albedo",VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,_renderCommandPool,3);
    createFrameBufferAttachment("depth",DepthResource::findDepthFormat(_vkSetup),VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,_renderCommandPool,4);
    
    std::array<VkAttachmentDescription, 5> attachmentDescriptions={};
    
    //swap chain attachment
    attachmentDescriptions[0]=createOutputColorAttachmentDescription();
    attachmentDescriptions[0].finalLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //g-buffer attachment
    for (size_t i=1;i<5;i++)
    {
        attachmentDescriptions[i].samples=VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions[i].loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescriptions[i].storeOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[i].stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[i].stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[i].initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
        if (i==4)
        {
            attachmentDescriptions[i].finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }else
        {
            attachmentDescriptions[i].finalLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }
    attachmentDescriptions[1].format=_attachments["position"].format;
    attachmentDescriptions[2].format=_attachments["normal"].format;
    attachmentDescriptions[3].format=_attachments["albedo"].format;
    attachmentDescriptions[4].format=_attachments["depth"].format;

    std::vector<VkAttachmentReference> colorReferences;
    colorReferences.push_back(createOutputColorAttachmentReference());
    colorReferences.push_back({1,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    colorReferences.push_back({2,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    colorReferences.push_back({3,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    VkAttachmentReference depthReferences={4,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    std::array<VkSubpassDescription, 2> subpassDescriptions{};
    subpassDescriptions[0].pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[0].pColorAttachments=&colorReferences[1];
    subpassDescriptions[0].colorAttachmentCount=static_cast<uint32_t>(colorReferences.size()-1);
    subpassDescriptions[0].pDepthStencilAttachment=&depthReferences;

    subpassDescriptions[1].pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[1].pColorAttachments=&colorReferences[0];
    subpassDescriptions[1].colorAttachmentCount=1;

    VkAttachmentReference inputReferences[3];
    inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    
    subpassDescriptions[1].inputAttachmentCount = 3;
    subpassDescriptions[1].pInputAttachments = inputReferences;

    VkSubpassDependency dependency{};
    dependency.srcSubpass=0;
    dependency.dstSubpass=1;
    dependency.srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask=VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.srcAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
    dependency.dependencyFlags=VK_DEPENDENCY_BY_REGION_BIT;
    

    VkRenderPassCreateInfo renderpassInfo{};
    renderpassInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassInfo.pAttachments=attachmentDescriptions.data();
    renderpassInfo.attachmentCount=static_cast<uint32_t>(attachmentDescriptions.size());
    renderpassInfo.subpassCount=static_cast<uint32_t>(subpassDescriptions.size());
    renderpassInfo.pSubpasses=subpassDescriptions.data();
    renderpassInfo.dependencyCount=1;
    renderpassInfo.pDependencies=&dependency;

    if (vkCreateRenderPass(_vkSetup->_device,&renderpassInfo,nullptr,&_deferredRenderPass)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create deferred render pass!");
    }
}

void DeferredRendering::createDeferredFramebuffer()
{
    std::array<VkImageView,5> attachmentViews;
    attachmentViews[0]=_attachments["output"].imageView;
    attachmentViews[1]=_attachments["position"].imageView;
    attachmentViews[2]=_attachments["normal"].imageView;
    attachmentViews[3]=_attachments["albedo"].imageView;
    attachmentViews[4]=_attachments["depth"].imageView;
    

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

    //refer to 1 color attachment
    VkColorComponentFlags colBlendAttachFlag=VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendAttachmentState colorBlendAttachment=utils::initPipelineColorBlendAttachmentState(colBlendAttachFlag,VK_FALSE);
    
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
    pipelineInfo.subpass=0;

    //final pipeline
    vertShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(FINAL_VERT_SHADER));
    fragShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(FINAL_FRAG_SHADER));

    shaderStages[0]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,vertShaderModule,"main");
    shaderStages[1]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,fragShaderModule,"main");

    rasterizer.cullMode=VK_CULL_MODE_FRONT_BIT;

    VkPipelineVertexInputStateCreateInfo emptyInputStateInfo=utils::initPipelineVertexInputStateCreateInfo(0,nullptr,0,nullptr);
    pipelineInfo.pVertexInputState=&emptyInputStateInfo;

    if (vkCreateGraphicsPipelines(_vkSetup->_device,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&_finalPipeline)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create final pipeline!");
    }
    
    vkDestroyShaderModule(_vkSetup->_device,vertShaderModule,nullptr);
    vkDestroyShaderModule(_vkSetup->_device,fragShaderModule,nullptr);

    //combine pipeline
    pipelineInfo.renderPass=_deferredRenderPass;
    pipelineInfo.subpass=1;
    //pipelineInfo.layout=
    
    vertShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(DFD_SUBPASS_LIGHTING_VERT_SHADER));
    fragShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(DFD_SUBPASS_LIGHTING_FRAG_SHADER));

    shaderStages[0]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,vertShaderModule,"main");
    shaderStages[1]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,fragShaderModule,"main");

    rasterizer.cullMode=VK_CULL_MODE_FRONT_BIT;

    //VkPipelineVertexInputStateCreateInfo emptyInputStateInfo=utils::initPipelineVertexInputStateCreateInfo(0,nullptr,0,nullptr);
    pipelineInfo.pVertexInputState=&emptyInputStateInfo;

    if (vkCreateGraphicsPipelines(_vkSetup->_device,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&_combinePipeline)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create combine pipeline!");
    }

    vkDestroyShaderModule(_vkSetup->_device,vertShaderModule,nullptr);
    vkDestroyShaderModule(_vkSetup->_device,fragShaderModule,nullptr);

    //gbuffer pipeline
    pipelineInfo.renderPass=_deferredRenderPass;
    pipelineInfo.subpass=0;

    vertShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(DFD_SUBPASS_GBUFFER_VERT_SHADER));
    fragShaderModule=Shader::createShaderModule(_vkSetup,Shader::readFile(DFD_SUBPASS_GBUFFER_FRAG_SHADER));

    shaderStages[0]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,vertShaderModule,"main");
    shaderStages[1]=utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,fragShaderModule,"main");

    rasterizer.cullMode=VK_CULL_MODE_BACK_BIT;

    auto bindingDescription = _model->getBindingDescriptions(0);
    auto attributeDescriptions = _model->getAttributeDescriptions(0);
    VkPipelineVertexInputStateCreateInfo vertexInputInfo=utils::initPipelineVertexInputStateCreateInfo(1,&bindingDescription,
        static_cast<uint32_t>(attributeDescriptions.size()),attributeDescriptions.data());

    pipelineInfo.pVertexInputState=&vertexInputInfo;

    //refer to 3 color attachment
    std::array<VkPipelineColorBlendAttachmentState,3> colorBlendAttachmentStates={
        utils::initPipelineColorBlendAttachmentState(colBlendAttachFlag,VK_FALSE),
        utils::initPipelineColorBlendAttachmentState(colBlendAttachFlag,VK_FALSE),
        utils::initPipelineColorBlendAttachmentState(colBlendAttachFlag,VK_FALSE)
    };

    colorBlending.attachmentCount=static_cast<uint32_t>(colorBlendAttachmentStates.size());
    colorBlending.pAttachments=colorBlendAttachmentStates.data();

    if (vkCreateGraphicsPipelines(_vkSetup->_device,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&_gbufferPipeline)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create combine pipeline!");
    }

    vkDestroyShaderModule(_vkSetup->_device,vertShaderModule,nullptr);
    vkDestroyShaderModule(_vkSetup->_device,fragShaderModule,nullptr);
}

void DeferredRendering::createDescriptorSets(){
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo=utils::initDescriptorSetAllocInfo(_descriptorPool,1,layouts.data());

    //gbuffer and lighting descriptor set
    if (vkAllocateDescriptorSets(_vkSetup->_device,&allocInfo,&_gbufferDescriptorSet)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to alloc gbuffer descriptor set!");
    }

    VkDescriptorBufferInfo gbufferUBOInfo{};
    gbufferUBOInfo.buffer=_vertUniformBuffer._buffer;
    gbufferUBOInfo.offset=0;
    gbufferUBOInfo.range=sizeof(UniformBufferObjectVert);
    if (!_vkSetup->isUniformBufferOffsetValid(gbufferUBOInfo.offset))
    {
        throw std::runtime_error("vertex uniform buffer do not fit data alignment!");
    }

    VkDescriptorImageInfo texDescriptor{};
    texDescriptor.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptor.imageView=_model->_textures[0]._textureImageView;
    texDescriptor.sampler=_model->_textures[0]._textureSampler;

    VkDescriptorImageInfo texDescriptorPosition{};
    texDescriptorPosition.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptorPosition.imageView=_attachments["position"].imageView;
    texDescriptorPosition.sampler=VK_NULL_HANDLE;

    VkDescriptorImageInfo texDescriptorNormal{};
    texDescriptorNormal.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptorNormal.imageView=_attachments["normal"].imageView;
    texDescriptorNormal.sampler=VK_NULL_HANDLE;

    VkDescriptorImageInfo texDescriptorAlbedo{};
    texDescriptorAlbedo.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptorAlbedo.imageView=_attachments["albedo"].imageView;
    texDescriptorAlbedo.sampler=VK_NULL_HANDLE;

    VkDescriptorBufferInfo fragDescriptor{};
    fragDescriptor.buffer=_fragUniformBuffer._buffer;
    fragDescriptor.offset=0;
    fragDescriptor.range=sizeof(UniformBufferObjectFrag);
    if (!_vkSetup->isUniformBufferOffsetValid(fragDescriptor.offset))
    {
        throw std::runtime_error("fragment uniform buffer do not fit data alignment!");
    }

    writeDescriptorSets={
        utils::initWriteDescriptorSet(_gbufferDescriptorSet,0,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,&gbufferUBOInfo),
        utils::initWriteDescriptorSet(_gbufferDescriptorSet,1,VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,&texDescriptorPosition),
        utils::initWriteDescriptorSet(_gbufferDescriptorSet,2,VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,&texDescriptorNormal),
        utils::initWriteDescriptorSet(_gbufferDescriptorSet,3,VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,&texDescriptorAlbedo),
        utils::initWriteDescriptorSet(_gbufferDescriptorSet,4,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,&fragDescriptor),
        utils::initWriteDescriptorSet(_gbufferDescriptorSet,5,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&texDescriptor)
    };

    vkUpdateDescriptorSets(_vkSetup->_device,static_cast<uint32_t>(writeDescriptorSets.size()),writeDescriptorSets.data(),0,nullptr);

    //final descriptor sets
    allocInfo.descriptorSetCount=static_cast<uint32_t>(layouts.size());
    _descriptorSets.resize(layouts.size());

    if (vkAllocateDescriptorSets(_vkSetup->_device,&allocInfo,_descriptorSets.data())!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorImageInfo texDescriptorutOutput{};
    texDescriptorutOutput.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptorutOutput.imageView=_attachments["output"].imageView;
    texDescriptorutOutput.sampler=_colorAttachmentSampler;

    //==================================================================================
    VkDescriptorImageInfo	descriptorImageInfos[9];

    for (uint32_t i = 0; i < _aodTextures.size(); ++i)
    {
        descriptorImageInfos[i].sampler = _colorAttachmentSampler;
        descriptorImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptorImageInfos[i].imageView = _aodTextures[i]._textureImageView;
    }

    //=================================================================================
    
    
    for (size_t i=0;i<_descriptorSets.size();i++)
    {
        writeDescriptorSets={
            utils::initWriteDescriptorSet(_descriptorSets[i],5,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&texDescriptorutOutput),
            utils::initWriteDescriptorSet(_descriptorSets[i],6,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,descriptorImageInfos,9),
        };

        vkUpdateDescriptorSets(_vkSetup->_device,static_cast<uint32_t>(writeDescriptorSets.size()),writeDescriptorSets.data(),0,nullptr);
    }
}

void DeferredRendering::updateUniformBuffers(uint32_t curImage){
    glm::mat4 proj=_camera->getProjectionMatrix(_swapChain->_extent.width/(float)_swapChain->_extent.height,0.1f,40.0f);
    proj[1][1]*=-1.0f;//y coordinates inverted, vulkan origin top left vs opengl bottom left
    glm::mat4 model=glm::mat4(1.0f);
   

    UniformBufferObjectVert vertUBO{};
    vertUBO.model=model;
    vertUBO.view=_camera->getViewMatrix();
    vertUBO.proj=proj;

    updateVertUniformBuffer(curImage,vertUBO);

    UniformBufferObjectFrag fragUBO{};
    fragUBO.viewPos=glm::vec4(_camera->getPosition().x,_camera->getPosition().y,_camera->getPosition().z,0.0);
    fragUBO.pointLights[0]={ {5.0f, 5.0f, 5.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 40.0f} };
    fragUBO.gap=glm::vec4(0.0);

    updateFragUniformBuffer(curImage,fragUBO);
}

void DeferredRendering::createCommandBuffer(){
    _gbufferCommandBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    _combineCommandBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    utils::createCommandBuffers(*_vkSetup,static_cast<uint32_t>(_gbufferCommandBuffer.size()),_gbufferCommandBuffer.data(),_renderCommandPool);
    utils::createCommandBuffers(*_vkSetup,static_cast<uint32_t>(_combineCommandBuffer.size()),_combineCommandBuffer.data(),_renderCommandPool);
}

void DeferredRendering::createSyncObjects(){
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _gbufferSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags=VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
    {
        if (vkCreateSemaphore(_vkSetup->_device,&semaphoreInfo,nullptr,&_imageAvailableSemaphores[i])!=VK_SUCCESS
            ||vkCreateSemaphore(_vkSetup->_device,&semaphoreInfo,nullptr,&_renderFinishedSemaphores[i])!=VK_SUCCESS
            ||vkCreateSemaphore(_vkSetup->_device,&semaphoreInfo,nullptr,&_gbufferSemaphores[i])!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }

        if (vkCreateFence(_vkSetup->_device,&fenceInfo,nullptr,&_inFlightFences[i])!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fences!");
        }
    }
}

void DeferredRendering::recordCommandBuffers()
{
    for (size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
    {
        recordGBufferCommandBuffer(_gbufferCommandBuffer[i]);
    }
}

void DeferredRendering::recordGBufferCommandBuffer(VkCommandBuffer cmdBuffer)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo=utils::initCommandBufferBeginInfo();

    //implicitly resets cmd buffer
    if (vkBeginCommandBuffer(cmdBuffer,&commandBufferBeginInfo)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    std::array<VkClearValue, 5> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[3].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[4].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass=_deferredRenderPass;
    renderPassBeginInfo.framebuffer=_deferredFrameBuffer;
    renderPassBeginInfo.renderArea.offset={0,0};
    renderPassBeginInfo.renderArea.extent=_swapChain->_extent;
    renderPassBeginInfo.clearValueCount=static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues=clearValues.data();

    //begin recording
    //subpass 0
    vkCmdBeginRenderPass(cmdBuffer,&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);

    //bind pipeline
    vkCmdBindPipeline(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_gbufferPipeline);

    //bind vertex buffer
    VkDeviceSize offset=0;
    vkCmdBindVertexBuffers(cmdBuffer,0,1,&_model->_vertexBuffer._buffer,&offset);

    //bind index buffer
    vkCmdBindIndexBuffer(cmdBuffer,_model->_indexBuffer._buffer,0,VK_INDEX_TYPE_UINT32);

    //bind descriptor
    vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_pipelineLayout,0,1,&_gbufferDescriptorSet,0,nullptr);

    //draw
    vkCmdDrawIndexed(cmdBuffer, _model->getIndicesNum(),1,0,0,0);

    //subpass 1
    vkCmdNextSubpass(cmdBuffer,VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_combinePipeline);

    vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_pipelineLayout,0,1,&_gbufferDescriptorSet,0,nullptr);

    //draw
    vkCmdDraw(cmdBuffer, 3,1,0,0);

    
    //end recording
    vkCmdEndRenderPass(cmdBuffer);

    if (vkEndCommandBuffer(cmdBuffer)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to end recording command buffer!");
    }
}

void DeferredRendering::recordRenderCommandBuffer(VkCommandBuffer cmdBuffer,uint32_t imgIndex)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo=utils::initCommandBufferBeginInfo();

    //implicitly resets cmd buffer
    if (vkBeginCommandBuffer(cmdBuffer,&commandBufferBeginInfo)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    std::array<VkClearValue,2> clearValues{};
    clearValues[0].color={0.0f,0.0f,0.0f,1.0f};
    clearValues[1].depthStencil={1.0f,0};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass=_outputRenderPass;
    renderPassBeginInfo.framebuffer=_backFrameBuffer._framebuffers[imgIndex];
    renderPassBeginInfo.renderArea.offset={0,0};
    renderPassBeginInfo.renderArea.extent=_swapChain->_extent;
    renderPassBeginInfo.clearValueCount=static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues=clearValues.data();

    //begin recording
    vkCmdBeginRenderPass(cmdBuffer,&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);

    //bind pipeline
    vkCmdBindPipeline(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_finalPipeline);

    //bind descriptor
    vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_pipelineLayout,0,1,&_descriptorSets[_currentFrame],0,nullptr);

    //draw
    vkCmdDraw(cmdBuffer, 3,1,0,0);
    
    //end recording
    vkCmdEndRenderPass(cmdBuffer);

    if (vkEndCommandBuffer(cmdBuffer)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to end recording command buffer!");
    }
}


void DeferredRendering::drawFrame()
{
    vkWaitForFences(_vkSetup->_device,1,&_inFlightFences[_currentFrame],VK_TRUE,UINT64_MAX);

    VkResult result=vkAcquireNextImageKHR(_vkSetup->_device,_swapChain->_swapChain,UINT64_MAX,
        _imageAvailableSemaphores[_currentFrame],VK_NULL_HANDLE,&_scImageIndex);

    if (result==VK_ERROR_OUT_OF_DATE_KHR)
    {
        //todo: recreate swapchain
        return;
    }else if (result!=VK_SUCCESS&&result!=VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffers(_currentFrame);
    recordRenderCommandBuffer(_combineCommandBuffer[_currentFrame],_scImageIndex);
    
    VkPipelineStageFlags waitStage=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};

    //gbuffer rendering
    submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask=&waitStage;

    submitInfo.waitSemaphoreCount=1;
    submitInfo.pWaitSemaphores=&_imageAvailableSemaphores[_currentFrame];

    submitInfo.commandBufferCount=1;
    submitInfo.pCommandBuffers=&_gbufferCommandBuffer[_currentFrame];

    submitInfo.signalSemaphoreCount=1;
    submitInfo.pSignalSemaphores=&_gbufferSemaphores[_currentFrame];

    if (vkQueueSubmit(_vkSetup->_graphicsQueue,1,&submitInfo,VK_NULL_HANDLE)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw gbuffer command buffer!");
    }

    //combine rendering
    submitInfo.waitSemaphoreCount=1;
    submitInfo.pWaitSemaphores=&_gbufferSemaphores[_currentFrame];

    submitInfo.signalSemaphoreCount=1;
    submitInfo.pSignalSemaphores=&_renderFinishedSemaphores[_currentFrame];

    submitInfo.commandBufferCount=1;
    submitInfo.pCommandBuffers=&_combineCommandBuffer[_currentFrame];

    vkResetFences(_vkSetup->_device,1,&_inFlightFences[_currentFrame]);

    if (vkQueueSubmit(_vkSetup->_graphicsQueue,1,&submitInfo,_inFlightFences[_currentFrame])!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw combined command buffer!");
    }
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount=1;
    presentInfo.pWaitSemaphores=&_renderFinishedSemaphores[_currentFrame];

    presentInfo.swapchainCount=1;
    presentInfo.pSwapchains=&_swapChain->_swapChain;
    presentInfo.pImageIndices=&_scImageIndex;

    result=vkQueuePresentKHR(_vkSetup->_presentQueue,&presentInfo);

   
    if (result==VK_ERROR_OUT_OF_DATE_KHR||result==VK_SUBOPTIMAL_KHR||_frameBufferResized)
    {
        _frameBufferResized=false;
        //todo:recreate swap chain
    }else if (result!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
    

    _currentFrame=(_currentFrame+1)%MAX_FRAMES_IN_FLIGHT;
    
    
}