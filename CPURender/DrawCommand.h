#pragma once
#include <glm/mat4x4.hpp>

struct Mesh;
struct Material;

struct DrawCommand
{
	Mesh const& mesh;
	glm::mat4x4 world;
	Material const& mat;
};

