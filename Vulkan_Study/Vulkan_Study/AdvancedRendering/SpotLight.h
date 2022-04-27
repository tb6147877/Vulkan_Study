#pragma once
#include "types.h"
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>


class SpotLight
{
public:
    //-Constructor----------------------------------------------------------------
    SpotLight(glm::vec3 dir={0.0f,0.0f,0.0f}, F32 n=0.0f,F32 f=0.0f)
        :_direction(dir),_nearZ(n),_farZ(f){}

    glm::mat4 getMVP(glm::mat4 model=glm::mat4(1.0f))
    {
        glm::mat4 proj = glm::perspective(glm::radians(45.0f),1.0f,_nearZ,_farZ);
        glm::mat4 view = glm::lookAt(_direction,{0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f});
        proj[1][1]*=-1.0f;
        proj[0][0]*=-1.0f;
        return proj*view*model;
    }

    glm::mat4 getView()
    {
        return glm::lookAt(_direction,{0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f});
    }
    
    //-Members---------------------------------------------------------------
    glm::vec3 _direction;
    F32 _nearZ;
    F32 _farZ;
};
