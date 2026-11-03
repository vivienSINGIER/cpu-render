#pragma once
#include <glm/glm.hpp>

#include <vector>

#include "../Heaps/Handle.hpp"
#include "../Shaders/ShaderBase/Material.h"
#include "../Core/FrameBuffer.hpp"
#include "../Maths/Transform.h"

#define MAX_PARTICLES 16'000

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

	float beginSize = 0;
	float endSize = 0;
};
 
struct alignas(16) ParticleDesc
{
	size_t amount = 100;
	float spawnRadius = 0.f;
	float particleBeginSize = 0.1f;
	float particleEndSize = 0.1f;

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

constexpr const char* notex = "NoTexture";

class ParticleManager
{
public:
	static ParticleManager* Instance();

	//floorDir et Ceildir are the limits of the random from each member of the vec3, example : if (-1, -1, -1) and (1, 1, 1) each value will be betweeen -1 and 1
	void Spawn(glm::vec3 _center, ParticleDesc& _desc, std::string texture_name = notex);

	std::string BindSlot(std::string _texture_name, std::string const& _path, bool _isSrgb = false, bool _genMips = true);

	ParticleEmitter* CreateEmitter(glm::vec3 _center, ParticleDesc& _desc, float _spawnDelay, std::string texture_name = notex);

	void Update(float dt);

	//Recommended to create a Particle Pass before with isCompute = true
	void Draw(Device* _pDevice);
private:
	static ParticleManager* m_pInstance;

	std::unordered_map<std::string, Handle> m_textures;

	std::unordered_map<std::string, std::vector<ParticleEmitter>> m_emitters;

	std::unordered_map<std::string, size_t> m_freeOffsets;

	std::unordered_map<std::string, Mesh> m_meshes;

	std::unordered_map<std::string, Handle> m_uav_buffers;
	std::unordered_map<std::string, int> m_max_particles;

	std::unordered_map<std::string, Material> m_unlitParticles;
	std::unordered_map<std::string, Material> m_particleMats;

	PassDesc m_particlesPass;

	Handle m_CS_handle;
	Transform identity;

	void Init();

	void BindSlot(std::string texture_name, Handle tex);
};