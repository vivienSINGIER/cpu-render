#pragma once
#include <glm/glm.hpp>

#include <vector>

#include "../Heaps/Handle.hpp"

struct alignas(64) UavParticle
{
	glm::vec3 pos = {0, 0, 0};
	float rotationRad = 0;

	glm::vec4 color = {0, 0, 0, 0};

	glm::vec3 dir = {0, 0, 0};
	float lifeTime = 0;

	float size = 0;
	float speed = 0;
};

class ParticleManager
{
public:
	Handle m_uavBuffer = {};

	static ParticleManager* Instance();

	void Spawn(glm::vec3 _spawn_pos, size_t _amount, float _radius, float _size, float _min_speed, float _max_speed, glm::vec4 _color, float _min_life_time, float _max_life_time);

private:
	static ParticleManager* m_pInstance;
};