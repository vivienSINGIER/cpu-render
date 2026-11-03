#pragma once
#include "../../main.h"
#include "../ShaderBase/Shader.h"
#include "../../Datas/Particle.h"

class ParticleCS : public ComputeShader
{
	SHADER_UAV_BUFF(UavParticle, data, 0)

public:
	static constexpr uint32_t GROUP_SIZE = 16;

	ParticleCS(GraphicsContext* _ctx) : ComputeShader(_ctx, { GROUP_SIZE, 1, 1 }) {}

	void Execute(glm::uvec3 _id) const override
	{
		// the last row of groups overhangs the screen when HEIGHT isn't a multiple of GROUP_SIZE
		if (_id.x >= MAX_PARTICLES)
			return;

		float dt = Pass().deltaTime;
		UavParticle p = Load_data(_id.x);

		if (p.lifeTime <= 0.f)
		{
			return;
		}

		p.pos += p.dir * p.speed * dt;
		p.lifeTime -= dt;
		p.rotationRad += p.rotSpeed * dt;

		float ratio = p.lifeTime / p.maxLifeTime;

		p.size = p.endSize + (p.beginSize - p.endSize) * ratio;
		p.color = p.endColor + (p.beginColor - p.endColor) * ratio;

		Store_data(_id.x, p);
	}
};

