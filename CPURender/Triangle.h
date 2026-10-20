#pragma once
#include <glm/glm.hpp>
#include "Color.h"

struct Vertex_IN
{
	glm::vec3 pos;
	Color color;
};
struct Vertex_OUT
{
	glm::vec3 pos;
	Color color;
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
Color GetPointColor(glm::vec2 _point, const Triangle& _tri);
glm::vec4 GetBoundingBox(const Triangle& _tri);
