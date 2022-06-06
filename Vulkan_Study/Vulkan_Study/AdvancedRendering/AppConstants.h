#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

const std::string APP_NAME = "Vulkan Study";
const std::string ENGINE_NAME = "Vulkan Engine";

const uint32_t WIDTH=1920;
const uint32_t HEIGHT=1080;

//path to the model
const std::string MODEL_PATH="D:/Project/Vulkan_Study/Vulkan_Study/Models/viking_room.obj";
const std::vector<std::string> MODEL_TEXTURES_PATH={"D:/Project/Vulkan_Study/Vulkan_Study/Textures/viking_room.png"};

//forward rendering shader path
const std::string FWD_VERT_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/ForwardVert.spv";
const std::string FWD_FRAG_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/ForwardFrag.spv";

//deferred rendering shader path
const std::string DFD_GBUFFER_VERT_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/DeferredGBufferVert.spv";
const std::string DFD_GBUFFER_FRAG_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/DeferredGBufferFrag.spv";
const std::string DFD_COMBINE_VERT_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/DeferredCombineVert.spv";
const std::string DFD_COMBINE_FRAG_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/DeferredCombineFrag.spv";

//deferred rendering with subpass shader path
const std::string DFD_SUBPASS_GBUFFER_VERT_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/DFD_Gbuffer_Vert.spv";
const std::string DFD_SUBPASS_GBUFFER_FRAG_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/DFD_Gbuffer_Frag.spv";
const std::string DFD_SUBPASS_LIGHTING_VERT_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/DFD_Lighting_Vert.spv";
const std::string DFD_SUBPASS_LIGHTING_FRAG_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/DFD_Lighting_Frag.spv";

//final shader path
const std::string FINAL_VERT_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/Final_Vert.spv";
const std::string FINAL_FRAG_SHADER="D:/Project/Vulkan_Study/Vulkan_Study/Shaders/Final_Frag.spv";

//aod test, pictures
const std::vector<std::string> AOD_PIC_PATHS={
    "D:/Project/Vulkan_Study/Vulkan_Study/Textures/AOD/0.png",
    "D:/Project/Vulkan_Study/Vulkan_Study/Textures/AOD/1.png",
    "D:/Project/Vulkan_Study/Vulkan_Study/Textures/AOD/2.png",
    "D:/Project/Vulkan_Study/Vulkan_Study/Textures/AOD/3.png",
    "D:/Project/Vulkan_Study/Vulkan_Study/Textures/AOD/4.png",
    "D:/Project/Vulkan_Study/Vulkan_Study/Textures/AOD/5.png",
    "D:/Project/Vulkan_Study/Vulkan_Study/Textures/AOD/6.png",
    "D:/Project/Vulkan_Study/Vulkan_Study/Textures/AOD/7.png",
    "D:/Project/Vulkan_Study/Vulkan_Study/Textures/AOD/9.png"
};

namespace Axes {
    // world axes
    const glm::vec3 WORLD_RIGHT = glm::vec3(-1.0f, 0.0f, 0.0f);
    const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 WORLD_FRONT = glm::vec3(0.0f, 0.0f, -1.0f);

    const glm::vec3 X = glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 Y = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 Z = glm::vec3(0.0f, 0.0f, 1.0f);

    // default directions
    const glm::vec3 RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 LEFT = glm::vec3(-1.0f, 0.0f, 0.0f);
    const glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 DOWN = glm::vec3(0.0f, -1.0f, 0.0f);
    const glm::vec3 FRONT = glm::vec3(0.0f, 0.0f, -1.0f);
    const glm::vec3 BACK = glm::vec3(0.0f, 0.0f, 1.0f);
}