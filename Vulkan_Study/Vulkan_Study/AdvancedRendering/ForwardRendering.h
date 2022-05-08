#pragma once
#include <glm/glm.hpp>


#include "SpotLight.h"
#include "RenderingBase.h"

class Camera;

class ForwardRendering:public RenderingBase
{
public:
    //-Initialisation and cleanup----------------------------------------
    virtual void initRenderer(VulkanSetup* pVkSetup, SwapChain* swapchain, Model* model) override;
    virtual void cleanupRenderer() override;

    //-Pipelines-----------------------------------------------------------------
    virtual void createPipeline() override;

    //-Descriptor initialisation functions--------------------------------------------
    virtual void createDescriptorSetLayout() override;
    virtual void createDescriptorSets() override;
    virtual void updateUniformBuffers(uint32_t curImage) override;

    //-Command buffer initialisation functions------------------------------------------------
    virtual void createCommandBuffer() override;
    virtual void recordCommandBuffers() override{}
    void recordRenderCommandBuffer(VkCommandBuffer cmdBuffer,uint32_t imgIndex);

    //-Sync structures-------------------------------------------------------------------
    virtual void createSyncObjects() override;
    
    //-Draw Frame-------------------------------------------------------
    virtual void drawFrame() override;

    void SetCamera(Camera* cam)
    {
        _camera=cam;
    }

    void SetPointLights(const std::vector<PointLight>& lights)
    {
        _pointLights[0]=lights[0];
    }
public:
    
    VkPipeline _pipeline;
    std::vector<VkCommandBuffer> _renderCommandBuffer;
    
    std::vector<VkDescriptorSet> _descriptorSets;

    //1 semaphore per frame, GPU-GPU sync
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    

    //1 fence per frame, CPU-GPU sync
    std::vector<VkFence> _inFlightFences;

    Camera* _camera;
    PointLight _pointLights[1];
    bool _frameBufferResized=false;

    glm::vec3 translate = glm::vec3(0.0f);
    glm::vec3 rotate = glm::vec3(0.0f);;
    float scale = 1.0f;
};

