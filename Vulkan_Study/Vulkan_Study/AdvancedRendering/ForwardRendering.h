#pragma once
#include "FrameBuffer.h"
#include "RenderingBase.h"

class ForwardRendering:public RenderingBase
{
public:
    //-Initialisation and cleanup----------------------------------------
    virtual void initRenderer(VulkanSetup* pVkSetup, SwapChain* swapchain, Model* model) override;
    virtual void cleanupRenderer() override;

    //-Render pass-------------------------------------------------------
    virtual void createRenderPass() override;

    //-Pipelines-----------------------------------------------------------------
    virtual void createPipeline() override;

    //-Descriptor initialisation functions--------------------------------------------
    virtual void createDescriptorSetLayout() override;

public:
    VkRenderPass _renderPass;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;
    VkCommandPool _renderCommandPool;
    std::vector<VkCommandBuffer> _renderCommandBuffer;
    BackFrameBuffer _backFrameBuffer;
};

