#pragma once
#include <map>
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
    void createFrameBufferAttachment(const std::string& name,VkFormat format,VkImageUsageFlagBits usage, const VkCommandPool& commandPool);

    //-Framebuffer-----------------------------------------------------------------
    void createDeferredFramebuffer();
    void createColorAttachmentSampler();
    
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
    virtual void createCommandBuffer() override;
    virtual void recordCommandBuffers() override{}

    //-Sync structures-------------------------------------------------------------------
    virtual void createSyncObjects() override;

    //-Draw Frame-------------------------------------------------------
    virtual void drawFrame() override;
public:
    //-Members------------------------------------------------------------------
    //std::vector<VkCommandBuffer> _renderCommandBuffer;
    VkRenderPass _deferredRenderPass;

    std::map<std::string, FrameBufferAttachment> _attachments;
    VkFramebuffer _deferredFrameBuffer;
    VkSampler _colorAttachmentSampler;
};
