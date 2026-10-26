#pragma once
#include "../../main.h"
#include "../ShaderBase/Shader.h"

class ParticleCS : public ComputeShader
{
	SHADER_UAV_TEX(target, 0)

public:
	static constexpr uint32_t GROUP_SIZE = 0;

	ParticleCS(GraphicsContext* _ctx) : ComputeShader(_ctx, { GROUP_SIZE, GROUP_SIZE, 1 }) {}

	void Execute(glm::uvec3 _id) const override
	{
	}
};

