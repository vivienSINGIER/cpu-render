#pragma once
#include <glm/glm.hpp>

#include <vector>

#include "../Heaps/Handle.hpp"

#define MAX_PARTICLES 32000

struct alignas(128) UavParticle
{
	glm::vec3 pos = {0, 0, 0};
	float rotationRad = 0;

	glm::vec4 beginColor = {0, 0, 0, 0};
	glm::vec4 endColor = { 0, 0, 0, 0 };

	glm::vec4 color = { 0, 0, 0, 0 };

	glm::vec3 dir = {0, 0, 0};
	float speed = 0;

	float rotSpeed = 0;
	float maxLifeTime = 0.001f;
	float lifeTime = 0;
	float size = 0;
};

struct ParticleEmitterDesc
{
	glm::vec3 spawnPos = { 0, 0, 0 };
	size_t maxParticles = 100;
	float spawnRadius = 0.f;
	float particleSize = 0.1f;
	float rotationSpeed = 0.f;
	
	glm::vec3 floorDir = { -1, -1, -1 };
	glm::vec3 ceilDir = { 1, 1, 1 };

	float minSpeed = 0.25f;
	float maxSpeed = 1.f;

	glm::vec4 beginColor = { 1, 0, 0, 1 };
	glm::vec4 endColor = { 1, 1, 0, 1 };

	float minLifeTime = 1.f;
	float maxLifeTime = 3.f;
};

class ParticleManager
{
public:
	Handle m_uavBuffer = {};

	static ParticleManager* Instance();

	//floorDir et Ceildir are the limits of the random from each member of the vec3, example : if (-1, -1, -1) and (1, 1, 1) each value will be betweeen -1 and 1
	void Spawn(ParticleEmitterDesc _desc);

private:
	static ParticleManager* m_pInstance;

	size_t m_freeOffset = 0;
};