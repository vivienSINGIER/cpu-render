#pragma once
#include "Mesh.h"
#include "../Shaders/ShaderBase/Material.h"
#include <glm/mat4x4.hpp>

#include "../Maths/Transform.h"
#include "../Maths/AABB.h"

class Device;

enum UpdateObjectType
{
	STATIC,
	DYNAMIC
};

enum ObjectType : uint8_t
{
	OBJECT,
	SKINNED
};

struct IObject
{
	size_t id = 0;

	ObjectType obj_type = ObjectType::OBJECT;
	UpdateObjectType update_type;

	Mesh mesh;

	Transform transform;
	MaterialData const* mat;
	uint8_t drawMask = 0b1111'1111;
};

struct Object : public IObject
{
	Object() { obj_type = ObjectType::OBJECT; }
};

