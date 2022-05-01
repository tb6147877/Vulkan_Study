#pragma once
#include "RenderingBase.h"

class ForwardRendering:RenderingBase
{
public:
    //virtual void createDescriptorSetLayout(const VulkanSetup& vkSetup) override;
    

    //-Initialisation and cleanup----------------------------------------
    virtual void initRenderer(VulkanSetup* pVkSetup, SwapChain* swapchain) override;
    virtual void cleanupRenderer() override;

    //-Render pass-------------------------------------------------------
    virtual void createRenderPass() override;

    //-Pipelines-----------------------------------------------------------------
    virtual void createPipeline() override;

public:
    VkRenderPass _renderPass;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;
};

