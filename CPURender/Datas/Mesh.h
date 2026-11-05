#pragma once
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

#include "Color.h"

#include "../Heaps/Handle.hpp"
#include "../Maths/AABB.h"

#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4

struct Vertex
{
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec2 uv = { 0.0f, 0.0f };
	glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
	glm::vec3 tangent = { 1.0f, 0.0f, 0.0f };
	glm::vec4 color = Color::WHITE;
	size_t id = 0;

	int bone_ids[MAX_BONE_INFLUENCE] = {-1};
	float weights[MAX_BONE_INFLUENCE] = {1.f};
};

struct Vertex_OUT
{
	glm::vec4 posH { 0.0f, 0.0f, 0.0f, 0.0f };
	glm::vec2 uv { 0.0f, 0.0f };
	glm::vec3 normal { 0.0f, 1.0f, 0.0f };
	glm::vec4 color = Color::WHITE;
	glm::vec3 tangent { 1.0f, 0.0f, 0.0f };
	glm::vec4 posModel { 0.0f, 0.0f, 0.0f, 0.0f };
	glm::vec4 posLightSpace { 0.0f, 0.0f, 0.0f, 0.0f };

	// uv change for one pixel step on screen, set per pixel by the rasterizer for mip selection
	glm::vec2 screenUV { 0.0f, 0.0f };
	glm::vec2 uvDdx { 0.0f, 0.0f };
	glm::vec2 uvDdy { 0.0f, 0.0f };
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

