#pragma once
#include <glm/glm.hpp>

#include "FrameBuffer.h"
#include "SpotLight.h"
#include "RenderingBase.h"



class ForwardRendering:public RenderingBase
{
public:
    struct UniformBufferObjectVert {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct UniformBufferObjectFrag {
        PointLight pointLights[1];
        glm::vec4 viewPos;
        glm::vec4 gap;
    };
    
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
    virtual void createUniformBuffers() override;
    virtual void createDescriptorPool() override;
    virtual void createDescriptorSets() override;
    void updateVertUniformBuffer(uint32_t imgIndex, const UniformBufferObjectVert& ubo);
    void updateFragUniformBuffer(uint32_t imgIndex, const UniformBufferObjectFrag& ubo);

public:
    VkRenderPass _renderPass;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;
    VkCommandPool _renderCommandPool;
    std::vector<VkCommandBuffer> _renderCommandBuffer;
    BackFrameBuffer _backFrameBuffer;

    VulkanBuffer _vertUniformBuffer;
    VulkanBuffer _fragUniformBuffer;

    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;
};

