#pragma once
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	float color[3];
};

struct Triangle
{
	Vertex v1;
	Vertex v2;
	Vertex v3;

	bool IsFront(bool _clockwise = false);
	bool IsInTriangle(glm::vec2 _point);
	glm::vec3 BarycentricWeight(glm::vec2 _point);
	uint32_t GetPointColor(glm::vec2 _point);
};

