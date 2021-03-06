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