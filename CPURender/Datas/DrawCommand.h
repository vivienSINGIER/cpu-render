#ifndef DRAW_COMMAND_H_DEFINED
#define DRAW_COMMAND_H_DEFINED

#include <glm/mat4x4.hpp>
#include "Mesh.h"

class MaterialData;

// TODO use handles for materials

struct DrawCommand
{
	Mesh mesh;
	glm::mat4x4 world;
	MaterialData const* material;
};

#endif

