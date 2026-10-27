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

void ParticleManager::Spawn(glm::vec3 _spawn_pos, size_t _amount, float _radius, glm::vec2 _size, float _min_speed, float _max_speed, glm::vec4 _color, float _min_life_time, float _max_life_time)
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

	for (int i = 0; i < _amount; i++)
	{
		Particle p;
		p.pos = glm::vec3(_spawn_pos.x + rand_range(-_radius, _radius), _spawn_pos.y + rand_range(-_radius, _radius), _spawn_pos.z + rand_range(-_radius, _radius));
		p.size = _size;
		p.speed = rand_range(_min_speed, _max_speed);
		p.color = _color;
		p.lifeTime = rand_range(_min_life_time, _max_life_time);

		//TODO EDIT
		p.rotationRad = 0.f;

		BufferFactory::Instance()->Write(m_uavBuffer, i, p);
	}
}
