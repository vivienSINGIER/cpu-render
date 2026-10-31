#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "AnimationData.h"

struct Node
{
	std::string name = "";
	glm::mat4x4 transform = {};
	Node* parent = nullptr;
	std::vector<Node*> children = {};
};

struct NodeAnim
{
	std::string name = "";
	std::vector<glm::vec3> positions = {};
	std::vector<glm::quat> rotations = {};
	std::vector<glm::vec3> scalings = {};
};

struct Animation
{
	double duration = 0.0;
	double ticks_per_sec = 0.0;

	std::vector<NodeAnim> channels = {};
};

struct AnimationPlayer
{
	Node* root_node = nullptr;
	std::vector<AnimMesh> meshes;
	std::vector<Animation> animations;
};

