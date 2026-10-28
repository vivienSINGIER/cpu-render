#pragma once
#include <glm/glm.hpp>

#include <vector>

#include "../Heaps/Handle.hpp"

#define MAX_PARTICLES 10'000

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
 
struct alignas(16) ParticleDesc
{
	size_t amount = 100;
	float spawnRadius = 0.f;
	float particleSize = 0.1f;

	glm::vec3 floorDir = { -1, -1, -1 };
	float minLifeTime = 1.f;

	glm::vec3 ceilDir = { 1, 1, 1 };
	float maxLifeTime = 3.f;

	glm::vec4 beginColor = { 1, 0, 0, 1 };
	glm::vec4 endColor = { 1, 1, 0, 1 };

	float rotationSpeed = 0.f;
	float minSpeed = 0.25f;
	float maxSpeed = 1.f;
};

struct ParticleEmitter
{
	ParticleDesc desc = {};
	glm::vec3 center = {};
	float spawnDelay = 0.5f;

	ParticleEmitter() = default;

	ParticleEmitter(glm::vec3 _center, ParticleDesc _desc, float _spawnDelay)
	{
		desc = _desc;
		center = _center;
		spawnDelay = _spawnDelay;
		spawnProgress = _spawnDelay;
	}
private:
	float spawnProgress = 0.f;


friend class ParticleManager;
};

class ParticleManager
{
public:
	Handle m_uavBuffer = {};

	static ParticleManager* Instance();

	//floorDir et Ceildir are the limits of the random from each member of the vec3, example : if (-1, -1, -1) and (1, 1, 1) each value will be betweeen -1 and 1
	void Spawn(glm::vec3 _center, ParticleDesc _desc);

	ParticleEmitter CreateEmitter(glm::vec3 _center, ParticleDesc _desc, float _spawnDelay);

	void UpdateEmitter(ParticleEmitter& pe, float dt);
private:
	static ParticleManager* m_pInstance;

	size_t m_freeOffset = 0;

};