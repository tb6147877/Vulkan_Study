/*
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
*/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Camera
{
public:
    enum MovementType {
        Forward,
        Back,
        Left,
        Right,
        Up,
        Down,
        Null
    };

    enum ProjectionType {
        Orthographic,
        Perspective
    };
private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;

    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_zoom;

    void updateVectors();
public:
    Camera(glm::vec3 pos = glm::vec3{0.0f,0.0f,3.0f},glm::vec3 worldUp= glm::vec3{ 0.0f,1.0f,0.0f },float yaw=-90.0f,float pitch=0.0f);
    ~Camera();

    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix(const float aspect, const float zNear=0.1f,const float zFar=100.0f);
    glm::mat4 getProjectionMatrix(const float left, const float right, const float bottom,const float top, const float zNear=0.1f, const float zFar=100.0f);
    float getZoom() const { return m_zoom; }
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getFront() const { return m_front; }
    void setMovementSpeed(const float speed) { m_movementSpeed = speed; }

    void processMouseScale(const float yOffset);
    void processKeyboard(const MovementType type, const float deltaTime);
    void processMouseMovement(const float xOffset, const float yOffset, const bool contrainPitch);
};
