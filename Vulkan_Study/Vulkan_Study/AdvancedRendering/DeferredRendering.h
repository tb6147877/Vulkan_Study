#pragma once
#include <map>

#include "Camera.h"
#include "RenderingBase.h"


class DeferredRendering:public RenderingBase
{
public:
    //-Frame buffer attachment---------------------------------------------------
    struct FrameBufferAttachment
    {
        VulkanImage vulkanImage{};
        VkFormat format=VK_FORMAT_UNDEFINED;
        VkImageView imageView=nullptr;
    };
    
public:
    //-Initialisation and cleanup----------------------------------------
    virtual void initRenderer(VulkanSetup* pVkSetup, SwapChain* swapchain, Model* model) override;
    virtual void cleanupRenderer() override;

    //-Renderpass---------------------------------------------------------------
    void createDeferredRenderPass();
    void createFrameBufferAttachment(const std::string& name,VkFormat format,VkImageUsageFlagBits usage,
        const VkCommandPool& commandPool,const int index=0);

    //-Framebuffer-----------------------------------------------------------------
    void createDeferredFramebuffer();
    void createColorAttachmentSampler();
    
    //-Pipelines-----------------------------------------------------------------
    virtual void createPipeline() override;

    //-Descriptor initialisation functions--------------------------------------------
    virtual void createDescriptorSetLayout() override;
    virtual void createDescriptorSets() override;
    virtual void updateUniformBuffers(uint32_t curImage) override;

    //-Command buffer initialisation functions------------------------------------------------
    virtual void createCommandBuffer() override;
    virtual void recordCommandBuffers() override;
    void recordGBufferCommandBuffer(VkCommandBuffer cmdBuffer);
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

private:
    void initAodDemo();
public:
    //-Members------------------------------------------------------------------
    std::vector<VkCommandBuffer> _gbufferCommandBuffer;
    std::vector<VkCommandBuffer> _combineCommandBuffer;
    VkRenderPass _deferredRenderPass;

    std::map<std::string, FrameBufferAttachment> _attachments;
    VkFramebuffer _deferredFrameBuffer;
    VkSampler _colorAttachmentSampler;

    VkPipeline _combinePipeline;
    VkPipeline _gbufferPipeline;
    VkPipeline _finalPipeline;

    //1 semaphore per frame, GPU-GPU sync
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkSemaphore> _gbufferSemaphores;

    //1 fence per frame, CPU-GPU sync
    std::vector<VkFence> _inFlightFences;

    std::vector<VkDescriptorSet> _descriptorSets;
    VkDescriptorSet _gbufferDescriptorSet;

    Camera* _camera;
    PointLight _pointLights[1];
    bool _frameBufferResized=false;

    glm::vec3 translate = glm::vec3(0.0f);
    glm::vec3 rotate = glm::vec3(0.0f);;
    float scale = 1.0f;

    //just for aod test
    std::vector<Texture> _aodTextures;
    
};
