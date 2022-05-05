/*#include "Camera.h"

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
}*/

#include "Camera.h"

Camera::Camera(glm::vec3 pos , glm::vec3 worldUp, float yaw , float pitch ) 
	:m_position{ pos }, m_worldUp{ worldUp }, m_yaw{ yaw }, m_pitch{ pitch }, m_front{ glm::vec3{0.0f,0.0f,-1.0f} }, m_movementSpeed{ 2.5f }, m_mouseSensitivity{ 0.1f }, m_zoom{ 45.0f }
{
	updateVectors();
}

Camera::~Camera() {

}

glm::mat4 Camera::getViewMatrix() {
	return glm::lookAt(m_position, m_position + m_front, m_up);
}



void Camera::updateVectors() {
	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw))*cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw))*cos(glm::radians(m_pitch));
	m_front = glm::normalize(front);
	m_right = glm::normalize(glm::cross(m_front, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::processMouseScale(const float yOffset) {
	m_zoom -= yOffset;
	if (m_zoom<1.0f)
	{
		m_zoom = 1.0f;
	}
	if (m_zoom>45.0f)
	{
		m_zoom = 45.0f;
	}
}

void Camera::processKeyboard(const MovementType type, const float deltaTime) {
	float velocity = m_movementSpeed * deltaTime;
	switch (type)
	{		
	case MovementType::Forward:
		m_position += velocity * m_front;
		break;
	case MovementType::Back:
		m_position -= velocity * m_front;
		break;
	case MovementType::Left:
		m_position -= velocity * m_right;
		break;
	case MovementType::Right:
		m_position += velocity * m_right;
		break;
	case MovementType::Up:
		m_position += velocity * m_up;
		break;
	case MovementType::Down:
		m_position -= velocity * m_up;
		break;
	case MovementType::Null:
		break;
	}
}

void Camera::processMouseMovement(const float xOffset, const float yOffset, const bool contrainPitch) {
	m_yaw += xOffset * m_mouseSensitivity;
	m_pitch += yOffset * m_mouseSensitivity;
	if (contrainPitch)
	{
		if (m_pitch>89.0f)
		{
			m_pitch = 89.0f;
		}
		if (m_pitch<-89.0f)
		{
			m_pitch = -89.0f;
		}
	}
	updateVectors();
}

glm::mat4 Camera::getProjectionMatrix(const float aspect, const float zNear, const float zFar) {
	return glm::perspective(glm::radians(m_zoom), aspect, zNear, zFar);
}

glm::mat4 Camera::getProjectionMatrix(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
	return glm::ortho(left, right, bottom, top, zNear, zFar);
}