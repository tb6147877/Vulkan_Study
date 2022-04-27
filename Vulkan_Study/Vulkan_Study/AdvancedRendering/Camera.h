#pragma once
#include <glm/vec3.hpp>

#include "Orientation.h"

// camera enums
enum class CameraMovement : unsigned char {
    PitchUp = 0x00,
    PitchDown = 0x10,
    RollLeft = 0x20,
    RollRight = 0x30,
    YawLeft = 0x40,
    YawRight = 0x50,
    Right = 0x60,
    Left = 0x70,
    Forward = 0x80,
    Backward = 0x90,
    Upward = 0xA0,
    Downward = 0xB0
};

class Camera 
{
public:
    //-Constructor-------------------------------------------------------------------------------
    Camera(glm::vec3 initPos=glm::vec3(0.0f),float initAngleSpeed=0.0f,float initPosSpeed=0.0f)
        :_position(initPos),_angleChangeSpeed(initAngleSpeed),_positionChangeSpeed(initPosSpeed){}

    //-Useful getters-----------------------------------------------------------------------------
    glm::mat4 getViewMatrix();
    Orientation getOrientation();

    //-Input-------------------------------------------------------------------------------------
    void processInput(CameraMovement camMove,float deltaTime);
    
public:
    //-Members----------------------------------------------------------------------------------
    glm::vec3 _position;
    Orientation _orientation;
    float _angleChangeSpeed;
    float _positionChangeSpeed;
};
