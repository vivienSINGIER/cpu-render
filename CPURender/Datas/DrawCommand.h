#ifndef DRAW_COMMAND_H_DEFINED
#define DRAW_COMMAND_H_DEFINED

#include <glm/mat4x4.hpp>
#include "Mesh.h"

struct DrawCommand
{
	Mesh mesh;
	glm::mat4x4 world;
	// Material mat;
};

#endif

