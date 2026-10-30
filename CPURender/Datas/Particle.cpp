#include "Particle.h"
#include <iostream>

#include "../Factories/BufferFactory.h"
#include <random>
#include <omp.h>

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
	auto rand_ratio = [&](int i) -> float
		{
			thread_local std::mt19937 t(i);
			std::uniform_real_distribution<float> d(0.f, 1.f);

			return d(t);
		};

	//return a random val in [min, max]
	auto rand_range = [&](int i, float min = -1.f, float max = 1.f) -> float
		{
			thread_local std::mt19937 t(i);
			return min + (max - min) * rand_ratio(i);
		};

	float spawnRadius = _desc.spawnRadius;

	#pragma omp parallel
	#pragma omp for
	for (int i = 0; i < _desc.amount; i++)
	{
		int current_index = (m_freeOffset + i) % MAX_PARTICLES;

		UavParticle p;
		p.pos = glm::vec3(_center.x + rand_range(i, -spawnRadius, spawnRadius), _center.y + rand_range(i, -spawnRadius, spawnRadius), _center.z + rand_range(i, -spawnRadius, spawnRadius));
		p.beginSize = _desc.particleBeginSize;
		p.endSize = _desc.particleEndSize;

		p.speed = rand_range(i, _desc.minSpeed, _desc.maxSpeed);
		p.beginColor = _desc.beginColor;
		p.endColor = _desc.endColor;

		p.lifeTime = rand_range(i, _desc.minLifeTime, _desc.maxLifeTime);
		p.maxLifeTime = p.lifeTime;

		//TODO EDIT
		p.dir = glm::normalize(glm::vec3(rand_range(i, _desc.floorDir.x, _desc.ceilDir.x), rand_range(i, _desc.floorDir.y, _desc.ceilDir.y), rand_range(i, _desc.floorDir.z, _desc.ceilDir.z)));
		p.rotationRad = 0.f;
		p.rotSpeed = _desc.rotationSpeed;

		BufferFactory::Instance()->Write(m_uavBuffer, current_index, p);
	}

	m_freeOffset = (m_freeOffset + _desc.amount) % MAX_PARTICLES;
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
