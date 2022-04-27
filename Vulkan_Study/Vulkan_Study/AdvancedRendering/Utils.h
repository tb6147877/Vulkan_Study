﻿#pragma once
#include <vector>
#include <optional>
#include <vulkan/vulkan_core.h>

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
}

