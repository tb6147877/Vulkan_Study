#pragma once
#include "Model.h"
#include "SwapChain.h"
#include "VulkanSetup.h"

class RenderingBase
{
public:
    //-Initialisation and cleanup----------------------------------------
    virtual void initRenderer(VulkanSetup* pVkSetup, SwapChain* swapchain, Model* model)=0;
    virtual void cleanupRenderer()=0;

    //-Render pass-------------------------------------------------------
    virtual void createRenderPass()=0;

    //-Pipelines-----------------------------------------------------------------
    virtual void createPipeline()=0;

    //-Descriptor initialisation functions--------------------------------------------
    virtual void createDescriptorSetLayout()=0;
    virtual void createUniformBuffers()=0;
    virtual void createDescriptorPool()=0;
    virtual void createDescriptorSets()=0;
    
    
public:
    //-Members-----------------------------------------------------------
    VkDescriptorSetLayout _descriptorSetLayout;
    VulkanSetup* _vkSetup;
    SwapChain* _swapChain;
    Model* _model;
};
