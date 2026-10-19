#pragma once
#include <glm/glm.hpp>

class Camera
{
public:
	glm::mat4x4 transform;

	void Translate(glm::vec3 _translation);
	void RotateYaw(float _angle);
	void RotatePitch(float _angle);
	void RotateRoll(float _angle);
};

