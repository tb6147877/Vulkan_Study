#pragma once
#include <vulkan/vulkan_core.h>

#include "VulkanSetup.h"

//The VulkanBuffer class is a wrapper class for a VkBuffer and its vkMemory for ease of access.
//It also contain some static helper functions for creating buffers and some buffer operations like copying buffers.
//The Buffer struct is a minimal struct that contains information on a buffer, essentially a pointer to some contiguous
//memory and the size of the buffer, useful for creating a vulkan buffer.

struct Buffer
{
    unsigned char* data;
    size_t size;
};

struct UniformBase
{
    virtual void update()=0;
};

class VulkanBuffer
{
    //-Buffer operation info structs-----------------------------------------------------------
public:
    struct CreateInfo
    {
        VkDeviceSize size=0;
        VkBufferUsageFlags usage=0;
        VkMemoryPropertyFlags properties=0;
        VulkanBuffer* pVulkanBuffer=nullptr;//ptr to the buffer object we want to create
    };

    struct CopyInfo
    {
        VkBuffer* pSrc=nullptr;
        VkBuffer* pDst=nullptr;
        VkBufferCopy copyRegion{};
    };

public:
    //-Buffer cleanup---------------------------------------------------------------
    void cleanupBufferData(const VkDevice& device);

    //-Buffer creation-------------------------------------------------------------
    static void createBuffer(const VulkanSetup* vkSetup, CreateInfo* bufferCreateInfo, VkSharingMode sharingMode);

    //-Buffer copying-------------------------------------------------------------------
    static void copyBuffer(const VulkanSetup* vkSetup, const VkCommandPool& commandPool, CopyInfo* bufferCopyInfo);
    static void copyBufferToImage(const VulkanSetup* vkSetup, const VkCommandPool& renderCommandPool,
        VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy>& regions);

    //-Buffer creation on GPU--------------------------------------------------------
    static void createDeviceLocalBuffer(const VulkanSetup* vkSetup, const VkCommandPool& commandPool,
        const Buffer& buffer, VulkanBuffer* vkBuffer, VkBufferUsageFlagBits usage);

    //-Utility  uniform buffer creation--------------------------------------------
    template<typename T>
    static void createUniformBuffer(const VulkanSetup* vkSetup, size_t imagesSize, VulkanBuffer* buffer, VkMemoryPropertyFlags proerties);
public:
    //the vulkan buffer handle and its memory
    VkBuffer _buffer=nullptr;
    VkDeviceMemory _memory=nullptr;
};

//Template definitions
    template<typename T>
    void VulkanBuffer::createUniformBuffer(const VulkanSetup* vkSetup, size_t imagesSize, VulkanBuffer* buffer, VkMemoryPropertyFlags proerties)
    {
        VulkanBuffer::CreateInfo createInfo{};
        createInfo.size=static_cast<VkDeviceSize>(imagesSize*sizeof(T));
        createInfo.usage=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        createInfo.properties=proerties;
        createInfo.pVulkanBuffer=buffer;//a large buffer containing the uniform data for each frame buffer image

        VulkanBuffer::createBuffer(vkSetup,&createInfo,VK_SHARING_MODE_EXCLUSIVE);
    }
