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
bool IsInTriangle(glm::vec3 _bary);
glm::vec3 BarycentricWeightPerspective(const glm::vec3& _bary, float _w1, float _w2, float _w3);
glm::vec3 BarycentricWeight(const glm::vec2& _point, const Triangle& _tri);
Vertex_OUT InterpolateVertex(const Vertex_OUT& _v1, const Vertex_OUT& _v2, float _t);
void InterpolateVertex(Vertex_OUT& _vertOut, glm::vec2 _point, const Triangle& _tri, glm::vec3 _bary);
glm::vec4 InterpolatePositionH(glm::vec2 _point, const Triangle& _tri, glm::vec3 _bary);
glm::vec4 GetBoundingBox(const Triangle& _tri);
