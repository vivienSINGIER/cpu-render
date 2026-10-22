#pragma once
#include <glm/glm.hpp>
#include "../Datas/Color.h"

#include "../Datas/Mesh.h"

struct Triangle
{
	Vertex_OUT v1;
	Vertex_OUT v2;
	Vertex_OUT v3;
};

bool IsFront(const Triangle& _tri, bool _clockwise = false);
bool IsInTriangle(glm::vec2 _point, const Triangle& _tri);
glm::vec3 BarycentricWeight(glm::vec2 _point, const Triangle& _tri);
glm::vec3 BarycentricWeight(glm::vec3 _point, const Triangle& _tri);
Vertex_OUT InterpolateVertex(glm::vec2 _point, const Triangle& _tri);
Vertex_OUT InterpolateVertex(glm::vec3 _point, const Triangle& _tri);
Color GetPointColor(glm::vec2 _point, const Triangle& _tri);
glm::vec4 GetBoundingBox(const Triangle& _tri);
