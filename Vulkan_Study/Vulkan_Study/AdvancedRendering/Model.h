﻿#pragma once
#include <string>
#include <vector>
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Image.h"

class Model
{
public:
    //-Vertex POD------------------------------------------------------------------
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 nor;
        glm::vec2 tex;

        bool operator==(const Vertex& other) const {
            return pos == other.pos && nor == other.nor && tex == other.tex;
        }
    };

public:
    //-Support model file formats------------------------------------------------------
    enum class FileExtension:unsigned char{
        OBJ=0x0,
        GLTF=0x1
    };

public:
    //-Load Model---------------------------------------------------------------------
    void loadModel(const std::string& path);
    void loadObjModel(const std::string& path);
    void loadGltfModel(const std::string& path);

    //-File utils----------------------------------------------------------------------
    FileExtension getExtension(const std::string& path);

    //-Binding and attribute descriptions----------------------------------------------
    VkVertexInputBindingDescription getBindingDescriptions(uint32_t primitiveNum);
    std::array<VkVertexInputAttributeDescription,3> getAttributeDescriptions(uint32_t primitiveNum);

private:
    //-Members--------------------------------------------------------------------
    glm::vec3 _center{0.0};
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    std::vector<Image> _textures;
    FileExtension _ext;
    bool loadStatus;
};
