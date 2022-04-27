#include "Camera.h"

glm::mat4 Camera::getViewMatrix()
{
    glm::mat4 rotate=glm::mat4_cast(_orientation._orientation);
    //compute rotation based on orientation and translation based on position
    return rotate*glm::translate(glm::mat4(1.0f),-_position);
}

Orientation Camera::getOrientation()
{
    return _orientation;
}


void Camera::processInput(CameraMovement camMove,float deltaTime)
{
    switch (camMove)
    {
    case CameraMovement::PitchUp:
        _orientation.applyRotation(Axes::X,-_angleChangeSpeed*deltaTime);
        break;
    case CameraMovement::PitchDown:
        _orientation.applyRotation(Axes::X, _angleChangeSpeed*deltaTime);
        break;
    case CameraMovement::RollRight:
        _orientation.applyRotation(Axes::Z, _angleChangeSpeed * deltaTime);
        break;
    case CameraMovement::RollLeft:
        _orientation.applyRotation(Axes::Z, -_angleChangeSpeed * deltaTime);
        break;
    case CameraMovement::YawLeft:
        _orientation.applyRotation(Axes::Y, -_angleChangeSpeed * deltaTime);
        break;
    case CameraMovement::YawRight:
        _orientation.applyRotation(Axes::Y, _angleChangeSpeed * deltaTime);
        break;
    case CameraMovement::Left:
        _position+=glm::rotate(_orientation._orientation,Axes::LEFT)*deltaTime*_positionChangeSpeed;
        break;
    case CameraMovement::Right:
        _position+=glm::rotate(_orientation._orientation,Axes::RIGHT)*deltaTime*_positionChangeSpeed;
        break;
    case CameraMovement::Forward:
        _position += glm::rotate(_orientation._orientation, Axes::FRONT) * deltaTime * _positionChangeSpeed;
        break;
    case CameraMovement::Backward:
        _position += glm::rotate(_orientation._orientation, Axes::BACK) * deltaTime * _positionChangeSpeed;
        break;
    case CameraMovement::Upward:
        _position += glm::rotate(_orientation._orientation, Axes::UP) * deltaTime * _positionChangeSpeed;
        break;
    case CameraMovement::Downward:
        _position += glm::rotate(_orientation._orientation, Axes::DOWN) * deltaTime * _positionChangeSpeed;
        break;
    }
}