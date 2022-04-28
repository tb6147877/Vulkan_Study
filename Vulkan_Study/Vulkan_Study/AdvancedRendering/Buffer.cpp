#include "Buffer.h"

#include <stdexcept>

#include "Utils.h"


void VulkanBuffer::cleanupBufferData(const VkDevice& device)
{
    vkDestroyBuffer(device,_buffer,nullptr);
    vkFreeMemory(device,_memory,nullptr);
}

void VulkanBuffer::createBuffer(const VulkanSetup* vkSetup, CreateInfo* bufferCreateInfo, VkSharingMode sharingMode)
{
    //fill in the corresponding struct
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size=bufferCreateInfo->size;//allocate a buffer of the right size in bytes
    bufferInfo.usage=bufferCreateInfo->usage;//what the data in the buffer is used for 
    bufferInfo.sharingMode=sharingMode;

    //attempt to create a buffer
    if (vkCreateBuffer(vkSetup->_device, &bufferInfo,nullptr,&bufferCreateInfo->pVulkanBuffer->_buffer)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    //created a buffer, but haven't assigned any memory yet, also get the right memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkSetup->_device,bufferCreateInfo->pVulkanBuffer->_buffer,&memRequirements);

    //allocate the memory for the buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize=memRequirements.size;
    allocInfo.memoryTypeIndex=utils::findMemoryType(&vkSetup->_physicalDevice,memRequirements.memoryTypeBits,bufferCreateInfo->properties);

    //allocate memory for the buffer, In a real world application, not supposed to actually call vkAllocateMemory for every
    //indicidual buffer.The maximum number of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical
    //device limit. The right way to allocate memory for large number of objects at the same time is to create a custom allocator
    //that splits up a single allocation along many different objects by using the offset parameters seen in other functions.
    if (vkAllocateMemory(vkSetup->_device,&allocInfo,nullptr,&bufferCreateInfo->pVulkanBuffer->_memory)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    //associate memory with buffer
    vkBindBufferMemory(vkSetup->_device,bufferCreateInfo->pVulkanBuffer->_buffer,bufferCreateInfo->pVulkanBuffer->_memory,0);
}

void VulkanBuffer::copyBuffer(const VulkanSetup* vkSetup, const VkCommandPool& commandPool, CopyInfo* bufferCopyInfo)
{
    //memory transfer operations are excuted using command buffers, like drawing commands. We need to allocate a temporary command buffer
    //could use a command pool for these short lived operations using the flag VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
    VkCommandBuffer commandBuffer=utils::beginSingleTimeCommands(&vkSetup->_device,commandPool);
    vkCmdCopyBuffer(commandBuffer, *bufferCopyInfo->pSrc, *bufferCopyInfo->pDst,1, &bufferCopyInfo->copyRegion);
    utils:utils::endSingleTimeCommands(&vkSetup->_device, &vkSetup->_graphicsQueue, &commandBuffer,&commandPool);
}

void VulkanBuffer::copyBufferToImage(const VulkanSetup* vkSetup, const VkCommandPool& renderCommandPool,
        VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy>& regions)
{
    //copying buffer to image
    VkCommandBuffer commandBuffer=utils::beginSingleTimeCommands(&vkSetup->_device,renderCommandPool);
    vkCmdCopyBufferToImage(commandBuffer,buffer,image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(regions.size()),regions.data());
    utils::endSingleTimeCommands(&vkSetup->_device,&vkSetup->_graphicsQueue,&commandBuffer,&renderCommandPool);
}

void VulkanBuffer::createDeviceLocalBuffer(const VulkanSetup* vkSetup, const VkCommandPool& commandPool,
        const Buffer& buffer, VulkanBuffer* vkBuffer, VkBufferUsageFlagBits usage)
{
    VulkanBuffer stagingBuffer;
    VulkanBuffer::CreateInfo createInfo{};
    createInfo.size=buffer.size;
    createInfo.usage=VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.properties=VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    createInfo.pVulkanBuffer=&stagingBuffer;

    VulkanBuffer::createBuffer(vkSetup, &createInfo,VK_SHARING_MODE_EXCLUSIVE);//buffers can be owned by a single queue or shared between many

    void* data;
    vkMapMemory(vkSetup->_device,stagingBuffer._memory,0,buffer.size,0,&data);
    memcpy(data,buffer.data,buffer.size);
    vkUnmapMemory(vkSetup->_device,stagingBuffer._memory);

    createInfo.usage=VK_BUFFER_USAGE_TRANSFER_DST_BIT|usage;
    createInfo.properties=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    createInfo.pVulkanBuffer=vkBuffer;

    VulkanBuffer::createBuffer(vkSetup, &createInfo,VK_SHARING_MODE_EXCLUSIVE);

    VkBufferCopy copyRegion{};
    copyRegion.size=buffer.size;
    copyRegion.srcOffset=0;
    copyRegion.dstOffset=0;

    VulkanBuffer::CopyInfo copyInfo{};
    copyInfo.pSrc=&stagingBuffer._buffer;
    copyInfo.pDst=&vkBuffer->_buffer;
    copyInfo.copyRegion=copyRegion;

    VulkanBuffer::copyBuffer(vkSetup, commandPool, &copyInfo);

    stagingBuffer.cleanupBufferData(vkSetup->_device);
}