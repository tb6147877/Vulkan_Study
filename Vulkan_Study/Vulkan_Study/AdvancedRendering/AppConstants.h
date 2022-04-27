#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

const std::string APP_NAME = "Vulkan Study";
const std::string ENGINE_NAME = "Vulkan Engine";

const uint32_t WIDTH=1024;
const uint32_t HEIGHT=768;

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