#include "ForwardRendering.h"
#include <array>
#include <stdexcept>

#include "AppConstants.h"
#include "DepthResource.h"
#include "Shader.h"
#include "Utils.h"
#include "Camera.h"


void ForwardRendering::initRenderer(VulkanSetup* pVkSetup,SwapChain* swapchain, Model* model)
{
    _vkSetup=pVkSetup;
    _swapChain=swapchain;
    _model=model;
    createCommandPool();
    createDescriptorSetLayout();
    createOutputRenderPass();
    _backFrameBuffer.initFramebuffer(_vkSetup,_swapChain,_renderCommandPool,_outputRenderPass);
    createPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createCommandBuffer();
    createSyncObjects();
    
}

void ForwardRendering::cleanupRenderer()
{
    RenderingBase::cleanupRenderer();
    vkDestroyCommandPool(_vkSetup->_device,_renderCommandPool,nullptr);
    
    vkDestroyDescriptorSetLayout(_vkSetup->_device,_descriptorSetLayout,nullptr);
    
    _backFrameBuffer.cleanupFramebuffers();
    vkDestroyPipelineLayout(_vkSetup->_device,_pipelineLayout,nullptr);
    vkDestroyPipeline(_vkSetup->_device,_pipeline,nullptr);

    for (size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
    {
        vkDestroySemaphore(_vkSetup->_device,_imageAvailableSemaphores[i],nullptr);
        vkDestroySemaphore(_vkSetup->_device,_renderFinishedSemaphores[i],nullptr);
        vkDestroyFence(_vkSetup->_device,_inFlightFences[i],nullptr);
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

    std::array<VkPipelineShaderStageCreateInfo,2> shaderStage{
        utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,vertShaderModule,"main"),
        utils::initPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,fragShaderModule,"main"),
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo=utils::initPipelineVertexInputStateCreateInfo(1,&bindingDescription,
        static_cast<uint32_t>(attributeDescriptions.size()),attributeDescriptions.data());
    VkPipelineInputAssemblyStateCreateInfo inputAssembly=utils::initPipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,VK_FALSE);
    VkPipelineViewportStateCreateInfo viewportState=utils::initPipelineViewportStateCreateInfo(1,&viewport,1,&scissor);
    VkPipelineRasterizationStateCreateInfo rasterizer=utils::initPipelineRasterStateCreateInfo(VK_POLYGON_MODE_FILL,VK_CULL_MODE_BACK_BIT,VK_FRONT_FACE_COUNTER_CLOCKWISE);
    VkPipelineMultisampleStateCreateInfo multisampling=utils::initPipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
    VkPipelineColorBlendStateCreateInfo colorBlending=utils::initPipelineColorBlendStateCreateInfo(1,&colorBlendAttachment);
    VkPipelineDepthStencilStateCreateInfo depthStencil=utils::initPipelineDepthStencilStateCreateInfo(VK_TRUE,VK_TRUE,VK_COMPARE_OP_LESS);
    VkPipelineLayoutCreateInfo pipelineLayoutInfo=utils::initPipelineLayoutCreateInfo(1,&_descriptorSetLayout);

    if (vkCreatePipelineLayout(_vkSetup->_device,&pipelineLayoutInfo,nullptr,&_pipelineLayout)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo=utils::initGraphicsPipelineCreateInfo(_pipelineLayout,_outputRenderPass);
    //fixed function pipeline
    pipelineInfo.pVertexInputState=&vertexInputInfo;
    pipelineInfo.pInputAssemblyState=&inputAssembly;
    pipelineInfo.pViewportState=&viewportState;
    pipelineInfo.pRasterizationState=&rasterizer;
    pipelineInfo.pMultisampleState=&multisampling;
    pipelineInfo.pColorBlendState=&colorBlending;
    pipelineInfo.pDepthStencilState=&depthStencil;
    pipelineInfo.stageCount=static_cast<uint32_t>(shaderStage.size());
    pipelineInfo.pStages=shaderStage.data();
    
    if (vkCreateGraphicsPipelines(_vkSetup->_device,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&_pipeline)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    
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

    utils::createDescriptorSetLayout(&_vkSetup->_device,&_descriptorSetLayout,setLayoutBindings);
}


void ForwardRendering::createDescriptorSets()
{
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo=utils::initDescriptorSetAllocInfo(_descriptorPool,static_cast<uint32_t>(layouts.size()),layouts.data());
    _descriptorSets.resize(layouts.size());

    if (vkAllocateDescriptorSets(_vkSetup->_device,&allocInfo,_descriptorSets.data())!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    

    VkDescriptorImageInfo texDescriptor{};
    texDescriptor.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texDescriptor.imageView=_model->_textures[0]._textureImageView;
    texDescriptor.sampler=_model->_textures[0]._textureSampler;
    
    
    for (size_t i=0;i<_descriptorSets.size();i++)
    {
        VkDescriptorBufferInfo vertDescriptor{};
        vertDescriptor.buffer=_vertUniformBuffer._buffer;
        vertDescriptor.offset=sizeof(UniformBufferObjectVert)*i;
        vertDescriptor.range=sizeof(UniformBufferObjectVert);
        if (!_vkSetup->isUniformBufferOffsetValid(vertDescriptor.offset))
        {
            throw std::runtime_error("vertex uniform buffer do not fit data alignment!");
        }

        VkDescriptorBufferInfo fragDescriptor{};
        fragDescriptor.buffer=_fragUniformBuffer._buffer;
        fragDescriptor.offset=sizeof(UniformBufferObjectFrag)*i;
        fragDescriptor.range=sizeof(UniformBufferObjectFrag);
        if (!_vkSetup->isUniformBufferOffsetValid(fragDescriptor.offset))
        {
            throw std::runtime_error("fragment uniform buffer do not fit data alignment!");
        }

        writeDescriptorSets={
            utils::initWriteDescriptorSet(_descriptorSets[i],0,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,&vertDescriptor),
            utils::initWriteDescriptorSet(_descriptorSets[i],1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&texDescriptor),
            utils::initWriteDescriptorSet(_descriptorSets[i],2,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,&fragDescriptor)
        };

        vkUpdateDescriptorSets(_vkSetup->_device,static_cast<uint32_t>(writeDescriptorSets.size()),writeDescriptorSets.data(),0,nullptr);
    }
}

void ForwardRendering::updateUniformBuffers(uint32_t curImage)
{
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

void ForwardRendering::createCommandBuffer(){
    _renderCommandBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    utils::createCommandBuffers(*_vkSetup,static_cast<uint32_t>(_renderCommandBuffer.size()),_renderCommandBuffer.data(),_renderCommandPool);
}

void ForwardRendering::createSyncObjects()
{
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags=VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
    {
        if (vkCreateSemaphore(_vkSetup->_device,&semaphoreInfo,nullptr,&_imageAvailableSemaphores[i])!=VK_SUCCESS
            ||vkCreateSemaphore(_vkSetup->_device,&semaphoreInfo,nullptr,&_renderFinishedSemaphores[i])!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }

        if (vkCreateFence(_vkSetup->_device,&fenceInfo,nullptr,&_inFlightFences[i])!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fences!");
        }
    }
}

void ForwardRendering::recordRenderCommandBuffer(VkCommandBuffer cmdBuffer,uint32_t imgIndex)
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
    vkCmdBindPipeline(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_pipeline);

    //bind vertex buffer
    VkDeviceSize offset=0;
    vkCmdBindVertexBuffers(cmdBuffer,0,1,&_model->_vertexBuffer._buffer,&offset);

    //bind index buffer
    vkCmdBindIndexBuffer(cmdBuffer,_model->_indexBuffer._buffer,0,VK_INDEX_TYPE_UINT32);

    //bind descriptor
    vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,_pipelineLayout,0,1,&_descriptorSets[_currentFrame],0,nullptr);

    //draw
    vkCmdDrawIndexed(cmdBuffer, _model->getIndicesNum(),1,0,0,0);
    
    //end recording
    vkCmdEndRenderPass(cmdBuffer);

    if (vkEndCommandBuffer(cmdBuffer)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to end recording command buffer!");
    }
}

void ForwardRendering::drawFrame()
{
    // will acquire an image from swap chain, exec commands in command buffer with images as attachments in the 
    // frameBuffer return the image to the swap buffer. These tasks are started simultaneously but executed 
    // asynchronously. However we want these to occur in sequence because each relies on the previous task success
    // For syncing we can use semaphores or fences and coordinate operations by having one operation signal another 
    // and another wait for a fence or semaphore to go from unsignaled to signaled.
    // We can access fence state with vkWaitForFences but not semaphores.
    // Fences are mainly for syncing app with rendering operations, used here to synchronise the frame rate.
    // Semaphores are for syncing operations within or across cmd queues. 
    // We want to sync queue operations to draw cmds and presentation, and we want to make sure the offscreen cmds
    // have finished before the final image composition using semaphores. 
    /*************************************************************************************************************/

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
    recordRenderCommandBuffer(_renderCommandBuffer[_currentFrame],_scImageIndex);

    VkPipelineStageFlags waitStage=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask=&waitStage;

    submitInfo.waitSemaphoreCount=1;
    submitInfo.pWaitSemaphores=&_imageAvailableSemaphores[_currentFrame];

    submitInfo.commandBufferCount=1;
    submitInfo.pCommandBuffers=&_renderCommandBuffer[_currentFrame];

    submitInfo.signalSemaphoreCount=1;
    submitInfo.pSignalSemaphores=&_renderFinishedSemaphores[_currentFrame];

    vkResetFences(_vkSetup->_device,1,&_inFlightFences[_currentFrame]);

    // submit the command buffer to the graphics queue, takes an array of submitinfo when work load is much larger
    // last param is a fence, which is signaled when the cmd buffer finishes executing and is used to inform that the frame has finished
    // being rendered (the commands were all executed). The next frame can start rendering!
    if (vkQueueSubmit(_vkSetup->_graphicsQueue,1,&submitInfo,_inFlightFences[_currentFrame])!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // submitting the result back to the swap chain to have it shown onto the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount=1;
    presentInfo.pWaitSemaphores=&_renderFinishedSemaphores[_currentFrame];

    presentInfo.swapchainCount=1;
    presentInfo.pSwapchains=&_swapChain->_swapChain;
    presentInfo.pImageIndices=&_scImageIndex;

    // submit request to put image from the swap chain to the presentation queue
    result=vkQueuePresentKHR(_vkSetup->_presentQueue,&presentInfo);

   
    if (result==VK_ERROR_OUT_OF_DATE_KHR||result==VK_SUBOPTIMAL_KHR||_frameBufferResized)
    {
        _frameBufferResized=false;
        //todo:recreate swap chain
    }else if (result!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
    
    // increment current frame
    _currentFrame=(_currentFrame+1)%MAX_FRAMES_IN_FLIGHT;
    
}