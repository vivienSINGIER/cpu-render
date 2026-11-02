#include "AnimationData.h"

#include "../stb_image.h"
#include <glm/glm.hpp>
#include <iostream>
#include <xmemory>

#include "AnimationPlayer.h"
#include "AssimpGLMHelpers.h"
#include "../Shaders/Shaders.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/ShaderFactory.h"
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
    m_materials.clear();

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
            vec.y = 1-mesh->mTextureCoords[0][i].y;
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

    std::string matName = material->GetName().C_Str();
    auto it = m_materials.find(matName);
    bool materialExists = it != m_materials.end();
    if (materialExists == false)
    {
        std::vector<Handle> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
        std::vector<Handle> metalnessMaps = loadMaterialTextures(material, aiTextureType_METALNESS);
        std::vector<Handle> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT);
        std::vector<Handle> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT);

        m_materials[matName] = ShaderFactory::Instance()->CreateMaterial("LitT_BP_Model");
        if (diffuseMaps.size() != 0) m_materials[matName]->SetTexture(LitT_BP_PS::TEX_SLOT_diffuse, diffuseMaps[0]);
        if (metalnessMaps.size() != 0) m_materials[matName]->SetTexture(LitT_BP_PS::TEX_SLOT_metal, metalnessMaps[0]);
        if (normalMaps.size() != 0) m_materials[matName]->SetTexture(LitT_BP_PS::TEX_SLOT_normal, normalMaps[0]);
        if (heightMaps.size() != 0) m_materials[matName]->SetTexture(LitT_BP_PS::TEX_SLOT_displacement, heightMaps[0]);

        //for (int i = 0; i < material->mNumProperties; i++)
        //{
        //    std::cout << material->mProperties[i]->mKey.C_Str();
        //    switch (material->mProperties[i]->mType)
        //    {
        //    case aiPTI_Buffer: std::cout << "Buffer"; break;
        //    case aiPTI_Double: std::cout << "Double"; break;
        //    case aiPTI_Float: std::cout << "Float"; break;
        //    case aiPTI_Integer: std::cout << "Int"; break;
        //    case aiPTI_String: std::cout << "String"; break;
        //    }
        //    std::cout << ":";
        //    for (int j = 0; j < material->mProperties[i]->mDataLength; j++)
        //    {
        //        std::cout << material->mProperties[i]->mData[j];
        //    }
        //    std::cout << std::endl;

        //    //if(material->mProperties[i]->mType == aiPTI_Double)
        //    //{
        //    //    //double res;
        //    //    memcpy(&res, material->mProperties[i]->mData, material->mProperties[i]->mDataLength);
        //    //    std::cout << material->mProperties[i]->mKey.C_Str() << ":" << res << std::endl;
        //    //}
        //    //if(material->mProperties[i]->mType == aiPTI_Float)
        //    //{
        //    //    //float res;
        //    //    memcpy(&res, material->mProperties[i]->mData, material->mProperties[i]->mDataLength);
        //    //    std::cout << material->mProperties[i]->mKey.C_Str() << ":" << res << std::endl;
        //    //}
        //    //if(material->mProperties[i]->mType == aiPTI_String)
        //    //{
        //    //    //std::string res;
        //    //   // memcpy(&res, material->mProperties[i]->mData, material->mProperties[i]->mDataLength);
        //    //    std::cout << material->mProperties[i]->mKey.C_Str() << ":" << res << std::endl;
        //    //}
        //    //if(material->mProperties[i]->mType == aiPTI_Integer)
        //    //{
        //    //    //int res;
        //    //    memcpy(&res, material->mProperties[i]->mData, material->mProperties[i]->mDataLength);
        //    //    std::cout << material->mProperties[i]->mKey.C_Str() << ":" << res << std::endl;
        //    //}
        //}

        LitT_BP_PS::MaterialCB cb = { glm::vec4(1, 1, 1, 1), glm::vec3(0, 0, 0), 0.5f, 1.0f, 0.5f };
        m_materials[matName]->SetParams(LitT_BP_PS::CB_SLOT_mat, cb);
    }

    m_materialKeys.push_back(matName);

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
std::vector<Handle> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
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
            Handle result = TextureFactory::Instance()->CreateTexture(str.C_Str(), str.C_Str());
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