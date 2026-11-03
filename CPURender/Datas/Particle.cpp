#include "Particle.h"
#include <iostream>
#include <random>
#include <omp.h>

#include "Mesh.h"

#include "../Factories/BufferFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Factories/ShaderFactory.h"
#include "../Shaders/ComputeShaders/ParticleCS.hpp"
#include "../Shaders/GraphicsShaders/UnlitParticle.hpp"
#include "../Shaders/Shaders.h"

#include "../Core/Device.h"

ParticleManager* ParticleManager::m_pInstance = nullptr;

ParticleManager* ParticleManager::Instance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new ParticleManager();
		m_pInstance->Init();
	}

	return m_pInstance;
}

void ParticleManager::Spawn(glm::vec3 _center, ParticleDesc _desc)
{
	if (m_uav_buffer.IsValid() == false)
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

		BufferFactory::Instance()->Write(m_uav_buffer, current_index, p);
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

void ParticleManager::Draw(Device* _pDevice)
{
	size_t count = MAX_PARTICLES;
	_pDevice->Dispatch(m_particleMat.get(), count / ParticleCS::GROUP_SIZE + 1, 1, 1);
	_pDevice->RegisterCommand(m_mesh, identity.GetMatrix(), m_unlitParticle.get());
}

void ParticleManager::Init()
{
	UnlitParticle_PS::ParticleColor color = { glm::vec4(1, 1, 1, 1) };
	m_uav_buffer = BufferFactory::Instance()->CreateUAVBuffer<UavParticle>(MAX_PARTICLES);
	m_CS_handle = ShaderFactory::Instance()->CreateComputeShader<ParticleCS>();

	m_particleMat = ShaderFactory::Instance()->CreateComputeMaterial("Particle");
	m_particleMat->SetUAVBuffer(0, m_uav_buffer);

	ShaderFactory::Instance()->RegisterGroup<UnlitParticle_VS, UnlitParticle_PS>("UnlitParticle");
	m_unlitParticle = ShaderFactory::Instance()->CreateMaterial("UnlitParticle");
	m_unlitParticle->SetUAVBuffer(0, m_uav_buffer);
	m_unlitParticle->SetParams<UnlitParticle_PS::ParticleColor>(UnlitParticle_PS::CB_SLOT_pcolor, color);

	BufferFactory::Instance()->Fill(m_uav_buffer, UavParticle());

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int k = 0; k < MAX_PARTICLES; k++)
	{
		for (int v = 0; v < 6; v++)
		{
			Vertex vert = Vertex();
			vert.id = (size_t)(k * 6 + v);

			vertices.push_back(vert);
		}

		indices.push_back(k * 6);
		indices.push_back(k * 6 + 2);
		indices.push_back(k * 6 + 1);

		indices.push_back(k * 6 + 3);
		indices.push_back(k * 6 + 1);
		indices.push_back(k * 6 + 2);
	}

	m_mesh = GeometryFactory::Instance()->CreateMesh(vertices, indices);
}

