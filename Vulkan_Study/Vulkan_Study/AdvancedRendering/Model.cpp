#include "Model.h"

#include <stdexcept>


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
    
}
void Model::loadGltfModel(const std::string& path)
{
    //todo: support gltf format
}

//-File utils----------------------------------------------------------------------
Model::FileExtension Model::getExtension(const std::string& path)
{
    
    throw std::runtime_error("Unsupported model format!");
}