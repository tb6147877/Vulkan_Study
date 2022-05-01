#include "DeferredRendering.h"

#include <set>
#include <stdexcept>

#include "Utils.h"


void DeferredRendering::createDescriptorSetLayout(const VulkanSetup& vkSetup)
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings={
        //binding 0: vertex shader uniform buffer
        utils::initDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
        //binding 1: position texture
        //A combined image sampler (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) is a single descriptor type
        //associated with both a sampler and an image resource, combining both a sampler and sampled
        //image descriptor into a single descriptor.
        utils::initDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 2: normal texture
        utils::initDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 3: albedo texture
        utils::initDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 4: fragment shader uniform buffer
        utils::initDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT),
        //binding 5: fragment shader shadow map
        utils::initDescriptorSetLayoutBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount=static_cast<uint32_t>(setLayoutBindings.size());
    layoutCreateInfo.pBindings=setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(vkSetup._device,&layoutCreateInfo,nullptr, &_descriptorSetLayout)!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}
