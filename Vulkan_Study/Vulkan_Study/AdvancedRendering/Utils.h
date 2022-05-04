#pragma once
#include <vector>
#include <optional>
#include <vulkan/vulkan_core.h>

class VulkanSetup;

#ifndef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

#ifdef VERBOSE
const bool enableVerboseValidation=true;
#else
const bool enableVerboseValidation=false;
#endif

const uint32_t DESCRIPTOR_POOL_NUM=10;

const std::vector<const char*> validationLayers={"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> deviceExtensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME};

namespace utils
{
    //-Command queue family info---------------------------------------------------------------------
    struct QueuefamilyIndices
    {
        std::optional<uint32_t> graphicsFamily; //queue supporting drawing commands
        std::optional<uint32_t> presentFamily; //queue for presenting image to vk surface
        inline bool isComplete()
        {
            return graphicsFamily.has_value()&&presentFamily.has_value();
        }

        static QueuefamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    };

    //-Memory type-------------------------------------------------------------------------------------
    uint32_t findMemoryType(const VkPhysicalDevice* physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    //-Begin and end single use commands
    VkCommandBuffer beginSingleTimeCommands(const VkDevice* device, const VkCommandPool& commandPool);
    void endSingleTimeCommands(const VkDevice* device,const VkQueue* queue, const VkCommandBuffer* commandBuffer, const VkCommandPool* commandPool);

    //-Texture operation info structs------------------------------------------------------------------
    bool hasStencilComponent(VkFormat format);

    //-Command buffer initialisation functions----------------------------------------------------------
    void createCommandPool(const VulkanSetup& vkSetup, VkCommandPool* commandPool, VkCommandPoolCreateFlags flags);
    void createCommandBuffers(const VulkanSetup& vkSetup, uint32_t count, VkCommandBuffer* commandBuffers, VkCommandPool& commandPool);

    //-Pipeline structs------------------------------------------------------------------------------
    VkPipelineVertexInputStateCreateInfo initPipelineVertexInputStateCreateInfo(
        uint32_t bindingCount,
        VkVertexInputBindingDescription* pVertexBindingDescriptions,
        uint32_t attributesCount,
        VkVertexInputAttributeDescription* pVertexAttributesDescriptions,
        VkPipelineVertexInputStateCreateFlags flags=0
    );

    VkPipelineShaderStageCreateInfo initPipelineShaderStageCreateInfo(
        VkShaderStageFlagBits stage,
        VkShaderModule& shader,
        const char* name
    );

    VkPipelineInputAssemblyStateCreateInfo initPipelineInputAssemblyStateCreateInfo(
        VkPrimitiveTopology topology,
        VkBool32 restartEnabled,
        VkPipelineInputAssemblyStateCreateFlags flags=0
    );

    VkPipelineRasterizationStateCreateInfo initPipelineRasterStateCreateInfo(
        VkPolygonMode polyMode,
        VkCullModeFlags cullMode,
        VkFrontFace frontFace,
        VkPipelineRasterizationStateCreateFlags flags=0,
        float lineWidth=1.0f
    );

    VkPipelineColorBlendStateCreateInfo initPipelineColorBlendStateCreateInfo(
        uint32_t attachmentCount,
        const VkPipelineColorBlendAttachmentState* pAttachment
    );

    VkPipelineDepthStencilStateCreateInfo initPipelineDepthStencilStateCreateInfo(
        VkBool32 depthTestEnable,
        VkBool32 depthWriteEnable,
        VkCompareOp depthCompareOp
    );

    VkPipelineViewportStateCreateInfo initPipelineViewportStateCreateInfo(
        uint32_t viewportCount,
        VkViewport* pViewports,
        uint32_t scissorCount,
        VkRect2D* pScissors,
        VkPipelineViewportStateCreateFlags flags=0
    );

    VkPipelineMultisampleStateCreateInfo initPipelineMultisampleStateCreateInfo(
        VkSampleCountFlagBits rasterizationSamples,
        VkPipelineMultisampleStateCreateFlags flags=0
    );

    VkPipelineLayoutCreateInfo initPipelineLayoutCreateInfo(
        uint32_t layoutCount,
        VkDescriptorSetLayout* pSetLayouts,
        VkPipelineLayoutCreateFlags flags=0
    );

    VkGraphicsPipelineCreateInfo initGraphicsPipelineCreateInfo(
        VkPipelineLayout layout,
        VkRenderPass renderpass,
        VkPipelineCreateFlags flags=0
    );

    VkPipelineColorBlendAttachmentState initPipelineColorBlendAttachmentState(
        VkColorComponentFlags mask,
        VkBool32 blendEnable
    );

    VkPipelineDynamicStateCreateInfo initPipelineDynamicStateCreateInfo(
        VkDynamicState* pDynamicStates,
        uint32_t dynamicStateCount,
        VkPipelineDynamicStateCreateFlags flags=0
    );
 
    
    //-Descriptor set struct--------------------------------------------------------------------------
    VkDescriptorSetLayoutBinding initDescriptorSetLayoutBinding(
        uint32_t binding,
        VkDescriptorType type,
        VkPipelineStageFlags flags=0
    );

    VkDescriptorSetAllocateInfo initDescriptorSetAllocInfo(
        VkDescriptorPool pool,
        uint32_t count,
        VkDescriptorSetLayout* pDesSetLayouts
    );

    VkWriteDescriptorSet initWriteDescriptorSet(
        VkDescriptorSet dst,
        uint32_t binding,
        VkDescriptorType type,
        VkDescriptorBufferInfo* pBufferInfo
    );

    VkWriteDescriptorSet initWriteDescriptorSet(
        VkDescriptorSet dst,
        uint32_t binding,
        VkDescriptorType type,
        VkDescriptorImageInfo* pImageInfo
    );

    //-Image structs-------------------------------------------------------------------
    VkImageViewCreateInfo initImageViewCreateInfo(VkImage image,VkImageViewType type, VkFormat format,
        VkComponentMapping componentMapping, VkImageSubresourceRange subResourceRange);
    
}


