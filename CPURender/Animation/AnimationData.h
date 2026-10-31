#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4

struct KeyPosition
{
	glm::vec3 position;
	float timeStamp;
};

struct KeyRotation
{
	glm::quat quat;
	float timeStamp;
};

struct KeyScale
{
	glm::vec3 scale;
	float timeStamp;
};

struct VertexWeight
{
	uint32_t vertex_id;
	float weight;
};

struct Bone
{
	std::string name;
	std::vector<VertexWeight> weights;
	glm::mat4x4 offset_matrix;
};

class AnimMesh
{
	std::vector<Bone> bones;
	std::vector<glm::vec3> vertices;
};

