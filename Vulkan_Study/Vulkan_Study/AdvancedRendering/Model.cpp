﻿#include "Model.h"
#include <unordered_map>
#include <stdexcept>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Assert.h"

//这是顶点去重的hash map所需的hash函数
namespace std {
    template<> struct hash<Model::Vertex> {
        size_t operator()(Model::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.nor) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.tex) << 1);
        }
    };
}


void Model::loadModel(const std::string& path)
{
    FileExtension ext=getExtension(path);
    switch (ext)
    {
    case FileExtension::OBJ:
        loadObjModel(path);
        return;
    /*case FileExtension::GLTF:
        loadGltfModel(path);
        return;*/
    }
    throw std::runtime_error("Unsupported model format!");
}
void Model::loadObjModel(const std::string& path)
{
    tinyobj::attrib_t attrib;//包含了顶点，法线，纹理坐标的数据
    std::vector<tinyobj::shape_t> shapes;//包含所有单独的obj和面，每个面由多个顶点数据组成的面，由index索引
    std::vector<tinyobj::material_t> materials;//材质，这里先不用
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }

    //用于顶点去重
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
			
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.nor = {
                attrib.normals[3*index.normal_index+0],
                attrib.normals[3*index.normal_index+1],
                attrib.normals[3*index.normal_index+2]
            };
			
            vertex.tex= {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1]
            };

            //add to the center of gravity
			_center+=vertex.pos;
			
            //顶点去重
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
                _vertices.push_back(vertex);
            }

            _indices.push_back(uniqueVertices[vertex]);
        }
    }
    //now compute the center by dividing by the number of vertices in the model
    _center/=(float)_vertices.size();
}
void Model::loadGltfModel(const std::string& path)
{
    //todo: support gltf format
}

//-File utils----------------------------------------------------------------------
Model::FileExtension Model::getExtension(const std::string& path)
{
    auto pos=path.find_last_of(".");
    m_assert(pos!=std::string::npos, "Invalid file path...");
    std::string extension=path.substr(pos+1);
    if (extension=="obj")
    {
        return FileExtension::OBJ;
    }
    else if (extension=="gltf")
    {
        return FileExtension::GLTF;
    }
    throw std::runtime_error("Unsupported model format!");
}