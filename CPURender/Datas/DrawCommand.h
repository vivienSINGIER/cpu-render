#ifndef DRAW_COMMAND_H_DEFINED
#define DRAW_COMMAND_H_DEFINED

#include <variant>
#include <glm/mat4x4.hpp>
#include "Mesh.h"

class MaterialData;

// TODO use handles for materials

struct DrawCommand
{
	Mesh mesh;
	glm::mat4x4 world;
	MaterialData const* material;
	std::vector<glm::mat4x4> finalBoneMatrices = {};
};

struct DispatchCommand
{
	MaterialData const* material;
	glm::uvec3 groups;
};

using Command = std::variant<DrawCommand, DispatchCommand>;

#endif

