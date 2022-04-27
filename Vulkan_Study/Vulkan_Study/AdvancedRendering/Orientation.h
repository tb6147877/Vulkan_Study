#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "AppConstants.h"

class Orientation
{
public:
    //-Constructor------------------------------------------------------------------
    Orientation():_orientation(0.0f,0.0f,0.0f,1.0f){}

public:
    //-Utility methods--------------------------------------------------------------
    inline void applyRotation(const glm::quat& rotation)
    {
        _orientation=glm::normalize(rotation*_orientation);
    }

    inline void applyRotation(const glm::vec3& axis, float angle)
    {
        _orientation=glm::normalize(glm::angleAxis(angle,axis)*_orientation);
    }

    inline void applyRotations(const glm::vec3& angles)
    {
        _orientation*=glm::angleAxis(angles.x,Axes::X*_orientation);
        _orientation*=glm::angleAxis(angles.y,Axes::Y*_orientation);
        _orientation*=glm::angleAxis(angles.z,Axes::Z*_orientation);
    }

    //todo:need to check the result of the func
    inline void rotateToOrientation(const Orientation& target)
    {
        _orientation=glm::inverse(target._orientation)*_orientation;
    }

    inline glm::mat4 toWorldSpaceRotation()
    {
        //return the orientation as a rotation in world space
        glm::mat4 rotation(1.0f);
        rotation[0]=glm::rotate(_orientation,glm::vec4(Axes::WORLD_RIGHT,0.0f));
        rotation[1]=glm::rotate(_orientation,glm::vec4(Axes::WORLD_UP,0.0f));
        rotation[2]=glm::rotate(_orientation,glm::vec4(Axes::WORLD_FRONT,0.0f));
        return rotation;
    }

    inline glm::mat4 toModelSpaceRotation(const glm::mat4& model)
    {
        //return the orientation as a rotation in model space
        glm::mat4 rotation(1.0f);
        rotation[0]=glm::rotate(_orientation,model[0]);
        rotation[1]=glm::rotate(_orientation,model[1]);
        rotation[2]=glm::rotate(_orientation,model[2]);
        return rotation;
    }

public:
    //-Members----------------------------------------------------------------------
    glm::quat _orientation;
};
