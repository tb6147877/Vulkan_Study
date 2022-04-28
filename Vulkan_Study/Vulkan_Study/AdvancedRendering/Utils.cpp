﻿#include "Utils.h"

#include <stdexcept>

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
}
