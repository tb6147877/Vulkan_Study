#pragma once
#include <string>
#include <vector>
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
        glm::vec4 tan;
        glm::vec2 tex;
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

private:
    //-Members--------------------------------------------------------------------
    glm::vec3 _center{0.0};
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    std::vector<Image> _textures;
    FileExtension _ext;
    bool loadStatus;
};
