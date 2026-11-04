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

#include "../Factories/TextureFactory.h"

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

std::string ParticleManager::BindSlot(std::string _texture_name, std::string const& _path, int max_particles, bool _isSrgb, bool _genMips)
{
	Handle h = TextureFactory::Instance()->CreateTexture(_texture_name, _path, _isSrgb, _genMips);
	BindSlot(_texture_name, h, max_particles);

	return _texture_name;
}

void ParticleManager::BindSlot(std::string texture_name, Handle tex, int max_particles)
{
	auto id = texture_name;
	m_textures[id] = tex;

	if (m_uav_buffers.contains(id))
		assert(false && "This ParticleManager slot has already been binded");

	m_uav_buffers[id] = BufferFactory::Instance()->CreateUAVBuffer<UavParticle>(max_particles);
	m_max_particles[id] = max_particles;

	m_particleMats[id] = ShaderFactory::Instance()->CreateComputeMaterial("Particle");
	m_particleMats[id]->SetUAVBuffer(ParticleCS::UAV_SLOT_data, m_uav_buffers[id]);
	m_particleMats[id]->SetParams(ParticleCS::CB_SLOT_cb_particles, ParticleCS::ParticleCS_CB{max_particles});

	m_unlitParticles[id] = ShaderFactory::Instance()->CreateMaterial("UnlitParticle");
	m_unlitParticles[id]->SetUAVBuffer(ParticleCS::UAV_SLOT_data, m_uav_buffers[id]);

	if (tex.IsValid())
	{
		m_unlitParticles[id]->SetTexture(UnlitParticle_PS::TEX_SLOT_diffuse, tex);
	}
	
	BufferFactory::Instance()->Fill(m_uav_buffers[id], UavParticle());

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int k = 0; k < max_particles; k++)
	{
		for (int v = 0; v < 6; v++)
		{
			Vertex vert = Vertex();
			vert.id = (size_t)(k * 6 + v);

			switch (v)
			{
			case 0:
				vert.uv = { 0.f, 1.f };
				break;
			case 3:
				vert.uv = { 1.f, 0.f };
				break;
			case 1:
			case 5:
				vert.uv = { 1.f, 1.f };
				break;
			case 2:
			case 4:
				vert.uv = { 0.f, 0.f };
				break;
			}

			vertices.push_back(vert);
		}

		indices.push_back(k * 6);
		indices.push_back(k * 6 + 2);
		indices.push_back(k * 6 + 1);

		indices.push_back(k * 6 + 3);
		indices.push_back(k * 6 + 1);
		indices.push_back(k * 6 + 2);
	}

	m_meshes[id] = GeometryFactory::Instance()->CreateMesh(vertices, indices);
}

ParticleEmitter* ParticleManager::CreateEmitter(glm::vec3 _center, ParticleDesc& _desc, float _spawnDelay, std::string _texture_name)
{
	auto id = _texture_name;

	m_emitters[id].push_back(ParticleEmitter());
	ParticleEmitter* current = &m_emitters[id].back();

	current->center = _center;
	current->desc = _desc;
	current->spawnDelay = _spawnDelay;

	return current;
}

void ParticleManager::Spawn(glm::vec3 _center, ParticleDesc& _desc, std::string _texture_name)
{
	auto id = _texture_name;

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
		int current_index = (m_freeOffsets[id] + i) % m_max_particles[id];

		//Randoms
		float costheta = rand_range(i, -1, 1);
		float phi = rand_range(i, 0, 2.f*PI);
		float sintheta = sqrtf(1.f - costheta * costheta);
		float r = cbrt(rand_ratio(i)) * _desc.spawnRadius;

		glm::vec3 dir(sintheta * cosf(phi), sintheta * sinf(phi), costheta);

		UavParticle p;
		p.pos = _center + dir * r;

		p.spawn_point = _center;
		p.spawnRadius = _desc.spawnRadius;
		p.beginSize = _desc.particleBeginSize;
		p.endSize = _desc.particleEndSize;

		p.speed = rand_range(i, _desc.minSpeed, _desc.maxSpeed);
		p.acceleration = _desc.acceleration;
		p.gravity = _desc.gravity;

		p.beginColor = _desc.beginColor;
		p.endColor = _desc.endColor;

		p.lifeTime = rand_range(i, _desc.minLifeTime, _desc.maxLifeTime);
		p.maxLifeTime = p.lifeTime;
		p.fadeDistance = _desc.fadeDistance;
		p.fadeMinimum = _desc.fadeMinimum;
		p.destroyIfExitSpawnRadius = _desc.destroyIfExitSpawnRadius;

		//TODO EDIT
		p.dir = glm::normalize(glm::vec3(rand_range(i, _desc.floorDir.x, _desc.ceilDir.x), rand_range(i, _desc.floorDir.y, _desc.ceilDir.y), rand_range(i, _desc.floorDir.z, _desc.ceilDir.z)));
		p.rotationRad = 0.f;
		p.rotSpeed = _desc.rotationSpeed;

		BufferFactory::Instance()->Write(m_uav_buffers[id], current_index, p);
	}

	m_freeOffsets[id] = (m_freeOffsets[id] + _desc.amount) % m_max_particles[id];
}

void ParticleManager::Update(float dt)
{
	for (auto& [key, emitters] : m_emitters)
	{
		for (auto& e : emitters)
		{
			if (e.spawnProgress < e.spawnDelay)
			{
				e.spawnProgress += dt;
				continue;
			}

			Spawn(e.center, e.desc, key);
			e.spawnProgress = 0.f;
		}
	}
}

void ParticleManager::Draw(Device* _pDevice)
{
	for (auto& [id, tex] : m_textures)
	{
		_pDevice->Dispatch(m_particleMats[id].get(), m_max_particles[id] / ParticleCS::GROUP_SIZE + 1, 1, 1);
		_pDevice->RegisterCommand(m_meshes[id], identity.GetMatrix(), m_unlitParticles[id].get());
	}
}

void ParticleManager::Init()
{
	m_CS_handle = ShaderFactory::Instance()->CreateComputeShader<ParticleCS>();
	ShaderFactory::Instance()->RegisterGroup<UnlitParticle_VS, UnlitParticle_PS>("UnlitParticle");

	BindSlot(std::string(notex), Handle(), MAX_PARTICLES_NOTEX);
}

