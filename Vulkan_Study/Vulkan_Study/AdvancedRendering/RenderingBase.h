#pragma once
#include "SwapChain.h"
#include "VulkanSetup.h"

class RenderingBase
{
public:
    //todo:拿出去
    //-Descriptor initialisation functions--------------------------------------------
    //virtual void createDescriptorSetLayout(const VulkanSetup& vkSetup)=0;

    //-Initialisation and cleanup----------------------------------------
    virtual void initRenderer(VulkanSetup* pVkSetup, SwapChain* swapchain)=0;
    virtual void cleanupRenderer()=0;

    //-Render pass-------------------------------------------------------
    virtual void createRenderPass()=0;

    //-Pipelines-----------------------------------------------------------------
    virtual void createPipeline()=0;
    
public:
    //-Members-----------------------------------------------------------
    //VkDescriptorSetLayout _descriptorSetLayout;
    VulkanSetup* _vkSetup;
    SwapChain* _swapChain;
};
