#include "Utils.h"

#include <functional>
#include <stdexcept>

#include "RenderingBase.h"
#include "VulkanSetup.h"

namespace utils
{
    QueuefamilyIndices QueuefamilyIndices::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueuefamilyIndices indices;

        uint32_t queueFamilyCount=0;
        vkGetPhysicalDeviceQueueFamilyProperties(device,&queueFamilyCount,nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device,&queueFamilyCount,queueFamilies.data());

        int i=0;
        for (const auto& queueFamily:queueFamilies)
        {
            if (queueFamily.queueFlags&VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily=i;
            }

            VkBool32 presentSupport=false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device,i,surface,&presentSupport);

            if (presentSupport)
            {
                indices.presentFamily=i;
            }

            if (indices.isComplete())
            {
                break;
            }
            i++;
        }
        return indices;
    }

    uint32_t findMemoryType(const VkPhysicalDevice* physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        //find best type of memory
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(*physicalDevice,&memProperties);
        //two arrays in the struct, memoryTypes and memoryHeaps. Heaps are distinct resources like VRAM and swap space in RAM
        //types exist within these heaps

        for (uint32_t i=0; i<memProperties.memoryTypeCount;i++)
        {
            //we want a memory type that is suitable for the buffer, but also able to write our data to memory
            if ((typeFilter&(1<<i))&&(memProperties.memoryTypes[i].propertyFlags&properties)==properties)
            {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    VkCommandBuffer beginSingleTimeCommands(const VkDevice* device, const VkCommandPool& commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool=commandPool;
        allocInfo.commandBufferCount=1;
        

        //allocate the command buffer
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(*device, &allocInfo, &commandBuffer);

        //set the struct for the command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        //start recording the command buffer
        vkBeginCommandBuffer(commandBuffer,&beginInfo);
        return commandBuffer;
    }
    
    void endSingleTimeCommands(const VkDevice* device,const VkQueue* queue, const VkCommandBuffer* commandBuffer, const VkCommandPool* commandPool)
    {
        //end recording
        vkEndCommandBuffer(*commandBuffer);

        //excute the command buffer by completing the submitinfo struct
        VkSubmitInfo submitInfo{};
        submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount=1;
        submitInfo.pCommandBuffers=commandBuffer;

        //submit the queue for execution
        vkQueueSubmit(*queue, 1, &submitInfo, VK_NULL_HANDLE);

        //here we could use a fence to schedule multiple transfers simultaneously and wait for them to complete instead
        //of executing all at the same time, alternatively use wait for the queue to execute.
        vkQueueWaitIdle(*queue);

        // free the command buffer once the queue is no longer in use
        vkFreeCommandBuffers(*device,*commandPool, 1, commandBuffer);
    }

    bool hasStencilComponent(VkFormat format)
    {
        return format==VK_FORMAT_D32_SFLOAT_S8_UINT||format==VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void createCommandPool(const VulkanSetup& vkSetup, VkCommandPool* commandPool, VkCommandPoolCreateFlags flags)
    {
        QueuefamilyIndices queueFamilyIndices=QueuefamilyIndices::findQueueFamilies(vkSetup._physicalDevice,vkSetup._surface);
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex=queueFamilyIndices.graphicsFamily.value(); //the queue to submit to
        poolInfo.flags=flags;

        if (vkCreateCommandPool(vkSetup._device,&poolInfo,nullptr,commandPool)!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool");
        }
    }
    
    void createCommandBuffers(const VulkanSetup& vkSetup, uint32_t count, VkCommandBuffer* commandBuffers, VkCommandPool& commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        
    }

    VkDescriptorSetLayoutBinding initDescriptorSetLayoutBinding(
        uint32_t binding,
        VkDescriptorType type,
        VkPipelineStageFlags flags
    )
    {
        VkDescriptorSetLayoutBinding descSetLayoutBinding{};
        descSetLayoutBinding.descriptorType=type;
        descSetLayoutBinding.descriptorCount=1;//change manually later if needed
        descSetLayoutBinding.binding=binding;
        descSetLayoutBinding.stageFlags=flags;
        return descSetLayoutBinding;
    }

    VkDescriptorSetAllocateInfo initDescriptorSetAllocInfo(
        VkDescriptorPool pool,
        uint32_t count,
        VkDescriptorSetLayout* pDesSetLayouts
    )
    {
        VkDescriptorSetAllocateInfo descSetAllocInfo{};
        descSetAllocInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descSetAllocInfo.descriptorPool=pool;
        descSetAllocInfo.descriptorSetCount=count;
        descSetAllocInfo.pSetLayouts=pDesSetLayouts;
        return  descSetAllocInfo;
    }

    VkWriteDescriptorSet initWriteDescriptorSet(
        VkDescriptorSet dst,
        uint32_t binding,
        VkDescriptorType type,
        VkDescriptorBufferInfo* pBufferInfo
    )
    {
        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType=type;
        writeDescriptorSet.dstSet=dst;
        writeDescriptorSet.dstBinding=binding;
        writeDescriptorSet.descriptorCount=1;
        writeDescriptorSet.pBufferInfo=pBufferInfo;
        return writeDescriptorSet;
    }

    VkWriteDescriptorSet initWriteDescriptorSet(
        VkDescriptorSet dst,
        uint32_t binding,
        VkDescriptorType type,
        VkDescriptorImageInfo* pImageInfo
    )
    {
        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType=type;
        writeDescriptorSet.dstSet=dst;
        writeDescriptorSet.dstBinding=binding;
        writeDescriptorSet.descriptorCount=1;
        writeDescriptorSet.pImageInfo=pImageInfo;
        return writeDescriptorSet;
    }

    VkImageViewCreateInfo initImageViewCreateInfo(VkImage image,VkImageViewType type, VkFormat format,
        VkComponentMapping componentMapping, VkImageSubresourceRange subResourceRange)
    {
        VkImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image=image;
        viewCreateInfo.viewType=type;//image as 1/2/3D textures and cubemaps
        viewCreateInfo.format=format;
        viewCreateInfo.components=componentMapping;
        viewCreateInfo.subresourceRange=subResourceRange;
        return viewCreateInfo;
    }

    VkPipelineVertexInputStateCreateInfo initPipelineVertexInputStateCreateInfo(
       uint32_t bindingCount,
       VkVertexInputBindingDescription* pVertexBindingDescriptions,
       uint32_t attributesCount,
       VkVertexInputAttributeDescription* pVertexAttributesDescriptions,
       VkPipelineVertexInputStateCreateFlags flags
    )
    {
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.flags=flags;
        vertexInputInfo.vertexBindingDescriptionCount=bindingCount;
        vertexInputInfo.pVertexBindingDescriptions=pVertexBindingDescriptions;
        vertexInputInfo.vertexAttributeDescriptionCount=attributesCount;
        vertexInputInfo.pVertexAttributeDescriptions=pVertexAttributesDescriptions;
        return vertexInputInfo;
    }

    VkPipelineShaderStageCreateInfo initPipelineShaderStageCreateInfo(
        VkShaderStageFlagBits stage,
        VkShaderModule& shader,
        const char* name
    )
    {
        VkPipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage=stage;
        shaderStageInfo.module=shader;
        shaderStageInfo.pName=name;
        return shaderStageInfo;
    }

    VkPipelineInputAssemblyStateCreateInfo initPipelineInputAssemblyStateCreateInfo(
        VkPrimitiveTopology topology,
        VkBool32 restartEnabled,
        VkPipelineInputAssemblyStateCreateFlags flags
    )
    {
        VkPipelineInputAssemblyStateCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        info.topology=topology;
        info.flags=flags;
        info.primitiveRestartEnable=restartEnabled;
        return info;
    }

    VkPipelineRasterizationStateCreateInfo initPipelineRasterStateCreateInfo(
        VkPolygonMode polyMode,
        VkCullModeFlags cullMode,
        VkFrontFace frontFace,
        VkPipelineRasterizationStateCreateFlags flags,
        float lineWidth
    )
    {
        VkPipelineRasterizationStateCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        info.polygonMode=polyMode;
        info.cullMode=cullMode;
        info.frontFace=frontFace;
        info.lineWidth=lineWidth;
        info.flags=flags;
        return info;
    }

    VkPipelineColorBlendStateCreateInfo initPipelineColorBlendStateCreateInfo(
        uint32_t attachmentCount,
        const VkPipelineColorBlendAttachmentState* pAttachment
    )
    {
        VkPipelineColorBlendStateCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        info.attachmentCount=attachmentCount;
        info.pAttachments=pAttachment;
        return info;
    }

    VkPipelineDepthStencilStateCreateInfo initPipelineDepthStencilStateCreateInfo(
        VkBool32 depthTestEnable,
        VkBool32 depthWriteEnable,
        VkCompareOp depthCompareOp
    )
    {
        VkPipelineDepthStencilStateCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        info.depthTestEnable=depthTestEnable;
        info.depthWriteEnable=depthWriteEnable;
        info.depthCompareOp=depthCompareOp;
        return info;
    }

    VkPipelineViewportStateCreateInfo initPipelineViewportStateCreateInfo(
        uint32_t viewportCount,
        VkViewport* pViewports,
        uint32_t scissorCount,
        VkRect2D* pScissors,
        VkPipelineViewportStateCreateFlags flags
    )
    {
        VkPipelineViewportStateCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        info.viewportCount=viewportCount;
        info.pViewports=pViewports;
        info.scissorCount=scissorCount;
        info.pScissors=pScissors;
        info.flags=flags;
        return info;
    }

    VkPipelineMultisampleStateCreateInfo initPipelineMultisampleStateCreateInfo(
        VkSampleCountFlagBits rasterizationSamples,
        VkPipelineMultisampleStateCreateFlags flags
    )
    {
        VkPipelineMultisampleStateCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        info.rasterizationSamples=rasterizationSamples;
        info.minSampleShading=1.0f;
        info.flags=flags;
        return info;
    }

    VkPipelineLayoutCreateInfo initPipelineLayoutCreateInfo(
       uint32_t layoutCount,
       VkDescriptorSetLayout* pSetLayouts,
       VkPipelineLayoutCreateFlags flags
    )
    {
        VkPipelineLayoutCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount=layoutCount;
        info.pSetLayouts=pSetLayouts;
        info.flags=flags;
        return info;
    }

    VkGraphicsPipelineCreateInfo initGraphicsPipelineCreateInfo(
        VkPipelineLayout layout,
        VkRenderPass renderpass,
        VkPipelineCreateFlags flags
    )
    {
        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
        graphicsPipelineCreateInfo.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.layout=layout;
        graphicsPipelineCreateInfo.renderPass=renderpass;
        graphicsPipelineCreateInfo.flags=flags;
        graphicsPipelineCreateInfo.basePipelineIndex=-1;
        graphicsPipelineCreateInfo.basePipelineHandle=VK_NULL_HANDLE;
        return graphicsPipelineCreateInfo;
    }

    VkPipelineColorBlendAttachmentState initPipelineColorBlendAttachmentState(
        VkColorComponentFlags mask,
        VkBool32 blendEnable
    )
    {
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask=mask;
        colorBlendAttachment.blendEnable=blendEnable;
        return colorBlendAttachment;
    }

    VkPipelineDynamicStateCreateInfo initPipelineDynamicStateCreateInfo(
        VkDynamicState* pDynamicStates,
        uint32_t dynamicStateCount,
        VkPipelineDynamicStateCreateFlags flags
    )
    {
        VkPipelineDynamicStateCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        info.pDynamicStates=pDynamicStates;
        info.dynamicStateCount=dynamicStateCount;
        info.flags=flags;
        return info;
    }
}
