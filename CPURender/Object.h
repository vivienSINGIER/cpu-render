#pragma once
#include "Mesh.h"
#include <glm/mat4x4.hpp>

#include "Transform.h"

struct Object
{
	Mesh mesh;
	Transform transform;
};

