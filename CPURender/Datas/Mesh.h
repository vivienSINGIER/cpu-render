#pragma once
#include <vector>
#include "Triangle.h"

#include "../Heaps/Handle.hpp"

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
	glm::vec3 position { 0.0f, 0.0f, 0.0f };
	glm::vec2 uv { 0.0f, 0.0f };
	glm::vec3 normal { 0.0f, 1.0f, 0.0f };
	glm::vec3 tangent { 1.0f, 0.0f, 0.0f };
	Color color = Color::WHITE;
};

using Index = uint32_t;

struct Mesh
{
	Handle vertexBuffer;
	uint32_t vertexCount;
	Handle indexBuffer;
	uint32_t indexCount;
};

