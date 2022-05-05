#pragma once
#include <glm/glm.hpp>

#include "FrameBuffer.h"
#include "SpotLight.h"
#include "RenderingBase.h"

class Camera;

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
    virtual void updateUniformBuffers(uint32_t curImage) override;
    void updateVertUniformBuffer(uint32_t imgIndex, const UniformBufferObjectVert& ubo);
    void updateFragUniformBuffer(uint32_t imgIndex, const UniformBufferObjectFrag& ubo);

    //-Command buffer initialisation functions------------------------------------------------
    virtual void createCommandPool() override;
    virtual void createCommandBuffer() override;
    virtual void recordCommandBuffers() override;
    void recordRenderCommandBuffer(uint32_t cmdBufferIndex);

    //-Sync structures-------------------------------------------------------------------
    virtual void createSyncObjects() override;
    
    //-Draw Frame-------------------------------------------------------
    virtual void drawFrame() override;

    void SetCamera(Camera* cam)
    {
        _camera=cam;
    }
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

    //1 semaphore per frame, GPU-GPU sync
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;

    //1 fence per frame, CPU-GPU sync
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;

    Camera* _camera;
    bool _frameBufferResized=false;

    glm::vec3 translate = glm::vec3(0.0f);
    glm::vec3 rotate = glm::vec3(0.0f);;
    float scale = 1.0f;
};

