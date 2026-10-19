#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

void Camera::Translate(glm::vec3 _translation)
{
	transform = glm::translate(transform, _translation);
}

void Camera::RotateYaw(float _angle)
{
	transform = glm::rotate(transform, _angle, glm::vec3(0, 1, 0));
}

void Camera::RotatePitch(float _angle)
{
	transform = glm::rotate(transform, _angle, glm::vec3(1, 0, 0));
}

void Camera::RotateRoll(float _angle)
{
	transform = glm::rotate(transform, _angle, glm::vec3(0, 0, 1));
}
