#include "Particle.h"
#include <iostream>

#include "../Factories/BufferFactory.h"

ParticleManager* ParticleManager::m_pInstance = nullptr;

ParticleManager* ParticleManager::Instance()
{
	if (m_pInstance == nullptr)
		m_pInstance = new ParticleManager();

	return m_pInstance;
}

void ParticleManager::Spawn(glm::vec3 _center, ParticleDesc _desc)
{
	if (m_uavBuffer.IsValid() == false)
	{
		std::cout << "Set UAV Buffer to use ParticleManager\n";
		return;
	}

	//return a random val between 0 and 1
	auto rand_ratio = [&]() -> float
		{
			return (float)((float)rand() / (float)RAND_MAX);
		};

	//return a random val in [min, max]
	auto rand_range = [&](float min = -1.f, float max = 1.f) -> float
		{
			return min + (max - min) * rand_ratio();
		};

	float spawnRadius = _desc.spawnRadius;

	for (int i = 0; i < _desc.amount; i++)
	{
		UavParticle p;
		p.pos = glm::vec3(_center.x +rand_range(-spawnRadius, spawnRadius), _center.y + rand_range(-spawnRadius, spawnRadius), _center.z + rand_range(-spawnRadius, spawnRadius));
		p.beginSize = _desc.particleBeginSize;
		p.endSize = _desc.particleEndSize;

		p.speed = rand_range(_desc.minSpeed, _desc.maxSpeed);
		p.beginColor = _desc.beginColor;
		p.endColor = _desc.endColor;

		p.lifeTime = rand_range(_desc.minLifeTime, _desc.maxLifeTime);
		p.maxLifeTime = p.lifeTime;

		//TODO EDIT
		p.dir = glm::normalize(glm::vec3(rand_range(_desc.floorDir.x, _desc.ceilDir.x), rand_range(_desc.floorDir.y, _desc.ceilDir.y), rand_range(_desc.floorDir.z, _desc.ceilDir.z)));
		p.rotationRad = 0.f;
		p.rotSpeed = _desc.rotationSpeed;

		BufferFactory::Instance()->Write(m_uavBuffer, m_freeOffset, p);
		m_freeOffset++;

		if(m_freeOffset >= MAX_PARTICLES)
			m_freeOffset %= MAX_PARTICLES;
	}
}

ParticleEmitter ParticleManager::CreateEmitter(glm::vec3 _center, ParticleDesc _desc, float _spawnDelay)
{
	return ParticleEmitter(_center, _desc, _spawnDelay);
}

void ParticleManager::UpdateEmitter(ParticleEmitter& pe, float dt)
{
	if (pe.spawnProgress < pe.spawnDelay)
	{
		pe.spawnProgress += dt;
		return;
	}

	Spawn(pe.center, pe.desc);
	pe.spawnProgress = 0.f;
}
