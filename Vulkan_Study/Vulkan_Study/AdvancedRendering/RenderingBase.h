#pragma once
#include "Model.h"
#include "SwapChain.h"
#include "VulkanSetup.h"
#include "SpotLight.h"
#include "FrameBuffer.h"
class RenderingBase
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
    virtual void initRenderer(VulkanSetup* pVkSetup, SwapChain* swapchain, Model* model)=0;
    virtual void cleanupRenderer();

    //-Render pass-------------------------------------------------------
    virtual void createOutputRenderPass();

    //-Pipelines-----------------------------------------------------------------
    virtual void createPipeline()=0;

    //-Descriptor initialisation functions--------------------------------------------
    virtual void createDescriptorSetLayout()=0;
    virtual void createUniformBuffers();
    virtual void createDescriptorPool();
    virtual void createDescriptorSets()=0;
    virtual void updateUniformBuffers(uint32_t curImage)=0;
    virtual void updateVertUniformBuffer(uint32_t imgIndex, const UniformBufferObjectVert& ubo);
    virtual void updateFragUniformBuffer(uint32_t imgIndex, const UniformBufferObjectFrag& ubo);

    //-Command buffer initialisation functions
    virtual void createCommandPool();
    virtual void createCommandBuffer()=0;
    virtual void recordCommandBuffers()=0;

    //-Sync structures-------------------------------------------------------------------
    virtual void createSyncObjects()=0;

    //-Draw Frame-------------------------------------------------------
    virtual void drawFrame()=0;
public:
    //-Members-----------------------------------------------------------
    VkDescriptorSetLayout _descriptorSetLayout;
    VulkanSetup* _vkSetup;
    SwapChain* _swapChain;
    Model* _model;
    size_t _currentFrame=0;
    uint32_t _scImageIndex=0;//index of current swap chain image

    BackFrameBuffer _backFrameBuffer;
    VkCommandPool _renderCommandPool;
    VkRenderPass _outputRenderPass;
    VkPipelineLayout _pipelineLayout;
    VulkanBuffer _vertUniformBuffer;
    VulkanBuffer _fragUniformBuffer;
    VkDescriptorPool _descriptorPool;
};
