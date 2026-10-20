#pragma once
#include <glm/glm.hpp>

struct Vertex_IN
{
	glm::vec3 pos;
	float color[3];
};
struct Vertex_OUT
{
	glm::vec3 pos;
	float color[3];
};

struct Triangle
{
	Vertex_OUT v1;
	Vertex_OUT v2;
	Vertex_OUT v3;
};

bool IsFront(const Triangle& _tri, bool _clockwise = false);
bool IsInTriangle(glm::vec2 _point, const Triangle& _tri);
glm::vec3 BarycentricWeight(glm::vec2 _point, const Triangle& _tri);
uint32_t GetPointColor(glm::vec2 _point, const Triangle& _tri);
glm::vec4 GetBoundingBox(const Triangle& _tri);
