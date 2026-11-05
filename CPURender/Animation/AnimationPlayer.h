#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>

#include "AnimationData.h"

struct SkinnedObject;

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

struct BoneInfo
{
	int id;
	glm::mat4 offset;
};

class Animation
{
public:
	Animation() = default;

	Animation(const std::string& animationPath, SkinnedObject* model);

	~Animation() = default;

	Bone* FindBone(const std::string& name);

	inline float GetTicksPerSecond() { return m_TicksPerSecond; }
	inline float GetDuration() { return m_Duration; }
	inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
	inline const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

private:
	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;

	void ReadMissingBones(const aiAnimation* animation, SkinnedObject* model);

	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
};

class Animator
{
public:
	Animator();

	void UpdateAnimation(float dt);

	void PlayAnimation(Animation* pAnimation);

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

	std::vector<glm::mat4> const& GetFinalBoneMatrices() const;

private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation* m_CurrentAnimation = nullptr;
	float m_CurrentTime;
	float m_DeltaTime;

};