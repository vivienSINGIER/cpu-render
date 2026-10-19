#pragma once
#include "Mesh.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Object
{
	Mesh mesh;
	glm::mat4x4 transform;

};

