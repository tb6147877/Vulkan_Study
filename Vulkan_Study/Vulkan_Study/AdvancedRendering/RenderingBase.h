#pragma once
#include "VulkanSetup.h"

class RenderingBase
{
public:
    //-Descriptor initialisation functions--------------------------------------------
    virtual void createDescriptorSetLayout(const VulkanSetup& vkSetup)=0;
    
public:
    //-Members-----------------------------------------------------------
    VkDescriptorSetLayout _descriptorSetLayout;
};
