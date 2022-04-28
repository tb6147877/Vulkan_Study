#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>

#include "Buffer.h"


//A wrapper for a VkImage and its associated view and memory. along with some helper image utility static functions


struct Image
{
    uint32_t width;
    uint32_t height;
    VkFormat format;
    Buffer imageData;
};
