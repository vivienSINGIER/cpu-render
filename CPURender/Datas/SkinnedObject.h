#pragma once
#include "Object.h"
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Animation/AnimationPlayer.h"

class SkinnedObject : public IObject
{
public:
    Animator animator;

    SkinnedObject(std::string const& path)
    {
        obj_type = ObjectType::SKINNED;
        loadModel(path);
    }

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

    void AddAnimation(std::string name, std::string const& path);
    Animation* GetAnimation(std::string name);
private:
    std::map<std::string, BoneInfo> m_BoneInfoMap; //
    std::unordered_map<std::string, Animation> m_animations;
    int m_BoneCounter = 0;

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const& path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene);

    void SetVertexBoneDataToDefault(Vertex& vertex);

    void processMesh(aiMesh* mesh, const aiScene* scene);

    void SetVertexBoneData(Vertex& vertex, int boneID, float weight);

    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
};
