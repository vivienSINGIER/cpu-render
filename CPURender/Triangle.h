#pragma once
#include <glm/glm.hpp>

#include "Color.h"

struct Vertex
{
	glm::vec3 pos;
	Color color;
};

struct Triangle
{
	Vertex v1;
	Vertex v2;
	Vertex v3;

	bool IsFront(bool _clockwise = false);
	bool IsInTriangle(glm::vec2 _point);
	glm::vec3 BarycentricWeight(glm::vec2 _point);
	Color GetPointColor(glm::vec2 _point);
	glm::vec4 GetBoundingBox();
};

