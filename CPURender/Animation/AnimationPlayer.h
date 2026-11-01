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
	std::vector<KeyPosition> mPositionKeys = {};
	std::vector<KeyRotation> mRotationKeys = {};
	std::vector<KeyScale> mScalingKeys = {};
};

struct Animation
{
	double duration = 0.0;
	double ticks_per_sec = 0.0;

	std::vector<NodeAnim> channels = {};
};
//
//struct AnimationScene
//{
//	Node* root_node = nullptr;
//	Model mesh;
//	std::vector<Animation> animations;
//};

