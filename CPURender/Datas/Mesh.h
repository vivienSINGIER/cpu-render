#pragma once
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

#include "Color.h"

#include "../Heaps/Handle.hpp"
#include "../Maths/AABB.h"

struct Vertex
{
	glm::vec3 position { 0.0f, 0.0f, 0.0f };
	glm::vec2 uv { 0.0f, 0.0f };
	glm::vec3 normal { 0.0f, 1.0f, 0.0f };
	glm::vec3 tangent { 1.0f, 0.0f, 0.0f };
	Color color = Color::WHITE;
};

struct Vertex_OUT
{
	glm::vec4 positionH { 0.0f, 0.0f, 0.0f, 0.0f };
	glm::vec4 positionModel { 0.0f, 0.0f, 0.0f, 0.0f };
	glm::vec2 uv { 0.0f, 0.0f };
	glm::vec3 normal { 0.0f, 1.0f, 0.0f };
	glm::vec3 tangent { 1.0f, 0.0f, 0.0f };
	Color color = Color::WHITE;
};

using Index = uint32_t;

struct Mesh
{
	Handle vertexBuffer;
	uint32_t vertexCount = 0;
	Handle indexBuffer;
	uint32_t indexCount = 0;

	AABB local_box;
	AABB world_box;
};

