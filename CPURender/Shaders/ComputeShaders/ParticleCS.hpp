#pragma once
#include "../../main.h"
#include "../ShaderBase/Shader.h"
#include "../../Datas/Particle.h"

class ParticleCS : public ComputeShader
{
	SHADER_UAV_BUFF(Particle, data, 0)

public:
	static constexpr uint32_t GROUP_SIZE = 16;

	ParticleCS(GraphicsContext* _ctx) : ComputeShader(_ctx, { GROUP_SIZE, 1, 1 }) {}

	void Execute(glm::uvec3 _id) const override
	{
		// the last row of groups overhangs the screen when HEIGHT isn't a multiple of GROUP_SIZE
		if (_id.x >= WIDTH || _id.y >= HEIGHT)
			return;

		//TODO CHANGE
		float dt = Pass().deltaTime;

		Particle p = Load_data(_id.x);

		if (p.lifeTime <= 0)
			return;

		p.pos += p.dir * p.speed * dt;
		p.lifeTime -= dt;

		Store_data(_id.x, p);
	}
};

