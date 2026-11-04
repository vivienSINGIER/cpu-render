#pragma once
#include "../../main.h"
#include "../ShaderBase/Shader.h"
#include "../../Datas/Particle.h"

class ParticleCS : public ComputeShader
{
	SHADER_UAV_BUFF(UavParticle, data, 0)

public:
	struct ParticleCS_CB
	{
		int max_particles;
	};

	SHADER_CB(ParticleCS_CB, cb_particles, CB_CUSTOM)

	static constexpr uint32_t GROUP_SIZE = 32;

	ParticleCS(GraphicsContext* _ctx) : ComputeShader(_ctx, { GROUP_SIZE, 1, 1 }) {}

	void Execute(glm::uvec3 _id) const override
	{
		// the last row of groups overhangs the screen when HEIGHT isn't a multiple of GROUP_SIZE
		if (_id.x >= cb_particles().max_particles)
			return;

		float dt = Pass().deltaTime;
		UavParticle p = Load_data(_id.x);

		if (p.lifeTime <= 0.f)
		{
			return;
		}

		if (p.destroyIfExitSpawnRadius)
		{
			glm::vec3 du = p.spawn_point - p.pos;
			float sqrDistExit = glm::dot(du, du);

			if (sqrDistExit > p.spawnRadius * p.spawnRadius)
			{
				p.lifeTime = 0.f;
				Store_data(_id.x, p);
				return;
			}
		}

		if(p.speed > 0)
			p.speed += p.acceleration * dt;

		p.pos += p.dir * p.speed * dt;
		p.pos.y -= p.gravity * dt;

		p.gravity += p.gravity * dt;

		p.lifeTime -= dt;
		p.rotationRad += p.rotSpeed * dt;

		float ratio = p.lifeTime / p.maxLifeTime;

		p.size = p.endSize + (p.beginSize - p.endSize) * ratio;
		p.color = p.endColor + (p.beginColor - p.endColor) * ratio;

		float alphaRatio = 1.f;
		if (p.fadeDistance > 0.f)
		{
			glm::vec3 vDist = Pass().eyePos - p.pos;

			float sqrDist = glm::dot(vDist, vDist);
			alphaRatio = clamp(1.f - sqrDist / (p.fadeDistance * p.fadeDistance), 0.05f, 1.f);
			p.color.a = alphaRatio;
			p.color.w = alphaRatio;
		}

		Store_data(_id.x, p);
	}
};

