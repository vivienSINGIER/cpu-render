#pragma once
#include "Mesh.h"
#include "../Shaders/ShaderBase/Material.h"
#include <glm/mat4x4.hpp>

#include "../Maths/Transform.h"
#include "../Maths/AABB.h"

class Device;

enum ObjectType
{
	STATIC,
	DYNAMIC
};

struct Object
{
	size_t id = 0;
	ObjectType type;

	Mesh mesh;
	Transform transform;
	MaterialData const* mat;
	uint8_t drawMask = 0b1111'1111;
};

