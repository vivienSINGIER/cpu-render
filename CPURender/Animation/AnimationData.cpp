#include "AnimationData.h"

#include "../stb_image.h"
#include <glm/glm.hpp>
#include <iostream>
#include <xmemory>

#include "AnimationPlayer.h"
#include "AssimpGLMHelpers.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"

Bone::Bone(const std::string& name, int ID, const NodeAnim* channel) :
    m_Name(name),
    m_ID(ID),
    m_LocalTransform(1.0f)
{
    m_NumPositions = (int)channel->mPositionKeys.size();
    for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
    {
        glm::vec3 position = channel->mPositionKeys[positionIndex].position;
        float timeStamp = channel->mPositionKeys[positionIndex].timeStamp;
        KeyPosition data;
        data.position = position;
        data.timeStamp = timeStamp;
        m_Positions.push_back(data);
    }

    m_NumRotations = (int)channel->mRotationKeys.size();
    for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
    {
        glm::quat orientation = channel->mRotationKeys[rotationIndex].orientation;
        float timeStamp = channel->mRotationKeys[rotationIndex].timeStamp;
        KeyRotation data;
        data.orientation = orientation;
        data.timeStamp = timeStamp;
        m_Rotations.push_back(data);
    }

    m_NumScalings = (int)channel->mScalingKeys.size();
    for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
    {
        glm::vec3 scale = channel->mScalingKeys[keyIndex].scale;
        float timeStamp = channel->mScalingKeys[keyIndex].timeStamp;
        KeyScale data;
        data.scale = scale;
        data.timeStamp = timeStamp;
        m_Scales.push_back(data);
    }
}

void Model::SetVertexBoneDataToDefault(Vertex& vertex)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        vertex.bone_ids[i] = -1;
        vertex.weights[i] = 0.0f;
    }
}

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::loadModel(std::string const& path)
{
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh m = processMesh(mesh, scene);
        meshes.push_back(m);
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<Handle> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        SetVertexBoneDataToDefault(vertex);

        vertex.position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
        vertex.normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.uv = vec;
        }
        else
            vertex.uv = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    Mesh m = GeometryFactory::Instance()->CreateMesh(vertices, indices);

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Handle> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Handle> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    std::vector<Handle> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    std::vector<Handle> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    ExtractBoneWeightForVertices(vertices, mesh, scene);

    return m;
}

void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (vertex.bone_ids[i] < 0)
        {
            
            vertex.weights[i] = weight;
            vertex.bone_ids[i] = boneID;
            break;
        }
    }
}

void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
{
    auto& boneInfoMap = m_BoneInfoMap;
    int& boneCount = m_BoneCounter;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCount;
            newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCount;
            boneCount++;
        }
        else
        {
            boneID = boneInfoMap[boneName].id;
        }
        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexId <= vertices.size());
            SetVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
 // the required info is returned as a Texture struct.
std::vector<Handle> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Handle> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(str.C_Str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            Handle result = TextureFactory::Instance()->CreateTexture(typeName, str.C_Str());
            textures.push_back(result);
            textures_loaded.push_back(result);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << str.C_Str() << std::endl;
            stbi_image_free(data);
        }
    }

    return textures;
}