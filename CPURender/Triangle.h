#pragma once
#include <glm/glm.hpp>
#include "Color.h"
#include <iostream>

struct Vertex_IN
{
	glm::vec3 pos;
	Color color;
};
struct Vertex_OUT
{
	glm::vec3 pos;
	Color color;

	//Vertex_OUT() :pos({0, 0, 0}), color(Color::BLACK) {};

	bool operator==(Vertex_OUT _other) // Temporary, TODO: do differently
	{
		return pos == _other.pos;
	}
};

struct Triangle
{
	std::shared_ptr<Vertex_OUT> v1;
	std::shared_ptr<Vertex_OUT> v2;
	std::shared_ptr<Vertex_OUT> v3;
};

bool IsFront(const Triangle& _tri, bool _clockwise = false);
bool IsInTriangle(glm::vec2 _point, const Triangle& _tri);
glm::vec3 BarycentricWeight(glm::vec2 _point, const Triangle& _tri);
Color GetPointColor(glm::vec2 _point, const Triangle& _tri);
Vertex_OUT InterpolateVertex(glm::vec2 _point, const Triangle& _tri);
glm::vec4 GetBoundingBox(const Triangle& _tri);
