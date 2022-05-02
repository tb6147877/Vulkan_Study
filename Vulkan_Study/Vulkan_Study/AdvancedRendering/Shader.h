#pragma once
#include <stdexcept>
#include <vector>
#include <fstream>
#include <string>

#include "VulkanSetup.h"


struct Shader
{
    inline static std::vector<char> readFile(const std::string& filename) {
        // create an input file stream, place cursor at the end and read in binary
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        // check that the stream was succesfully opened
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        // reading in the file at the end, we can use read position to determine how big the file is
        // and allocate a buffer accordingly. tellg returns position of current character (ate)
        size_t fileSize = (size_t)file.tellg();
        // now allocate the buffer to accomodate for the file size and the data (bytes)
        std::vector<char> buffer(fileSize);
        // place cursor back at the begining of the file
        file.seekg(0);
        // then read the data, pointer to vector data and fileSIze informs where to place data and how much to read
        file.read(buffer.data(), fileSize);
        // good practice, always close the file!
        file.close();
        // and return the file bytes in the buffer
        return buffer;
    }

    inline static VkShaderModule createShaderModule(VulkanSetup* vkSetup, const std::vector<char>& code)
    {
        //need to wrap the shader code into a shader module through this helper function, takes pointer to the
        //byte code as argument
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize=code.size();
        createInfo.pCode=reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(vkSetup->_device, &createInfo, nullptr, &shaderModule)!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module");
        }
        return shaderModule;
    }
};
