#ifndef SHADERS_LIT_4T_BP_HPP_DEFINED
#define SHADERS_LIT_4T_BP_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../Datas/Lights/LightHelpers.hpp"

using namespace Lighting;

class LitTerrain_BP_VS : public VertexShader
{
public:
    LitTerrain_BP_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    SHADER_TEX_OPT(texWeight, 0)

    inline Vertex_OUT Execute(Vertex const& _in) const override
    {
        Vertex_OUT v;
        v.color = texWeight(_in.uv);
        v.posModel = Object().model * glm::vec4({ _in.position, 1 });
        v.posH = v.posModel * Pass().viewProj;
        v.posLightSpace = v.posModel * Pass().lightSpace;
        v.normal = glm::mat3x3(Object().normalMatrix) * _in.normal;
        v.tangent = glm::mat3x3(Object().normalMatrix) * _in.tangent;
        v.uv = _in.uv;
        return v;
    }
};

class LitTerrain_BP_PS : public PixelShader
{
public:
    struct MaterialCB
    {
        glm::vec4 diffuseAlbedo;

        glm::vec3 fresnelR0;
        float roughness;

        float heightScale;
        float metalness;
        glm::vec2 uvWorldScale; // how many units is "one uv side"

        bool useTextures;
        float blendDistance;
    };

    SHADER_CB(MaterialCB, mat, CB_CUSTOM)
        SHADER_TEX_OPT(textureAtlas, 1)
        SHADER_TEX_OPT(ambient, 2)
        SHADER_TEX_OPT(shadow, 3)

        LitTerrain_BP_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    //float Depth(glm::vec2 _uv, glm::vec2 _ddx, glm::vec2 _ddy) const { return 1.0f - Sample_Grad_displacement(_uv, _ddx, _ddy).x; }

    float ShadowCalc(glm::vec4 posLightSpace) const
    {
        glm::vec3 projCoords = glm::vec3(posLightSpace) / posLightSpace.w;
        projCoords = projCoords * 0.5f + 0.5f;
        float closestDepth = shadow(glm::vec2(projCoords)).r;
        float currentDepth = projCoords.z;

        float bias = 0.005f;
        float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
        if (projCoords.z > 1.0)
            shadow = 0.0;

        return shadow;
    }

    glm::vec4 SampleDiff(int _materialIndex, float _atlasSideFactor, glm::vec2 _localUv, float _lod) const
    {
        return Sample_Lod_textureAtlas(_localUv + glm::vec2(((_materialIndex * 4) + 0) % 5, ((_materialIndex * 4) + 0) / 5) * _atlasSideFactor, _lod);
    }
    glm::vec4 SampleNorm(int _materialIndex, float _atlasSideFactor, glm::vec2 _localUv, float _lod) const
    {
        return Sample_Lod_textureAtlas(_localUv + glm::vec2(((_materialIndex * 4) + 1) % 5, ((_materialIndex * 4) + 1) / 5) * _atlasSideFactor, _lod);
    }
    glm::vec4 SampleDisp(int _materialIndex, float _atlasSideFactor, glm::vec2 _localUv, float _lod) const
    {
        return Sample_Lod_textureAtlas(_localUv + glm::vec2(((_materialIndex * 4) + 2) % 5, ((_materialIndex * 4) + 2) / 5) * _atlasSideFactor, _lod);
    }
    glm::vec4 SampleRough(int _materialIndex, float _atlasSideFactor, glm::vec2 _localUv, float _lod) const
    {
        return Sample_Lod_textureAtlas(_localUv + glm::vec2(((_materialIndex * 4) + 3) % 5, ((_materialIndex * 4) + 3) / 5) * _atlasSideFactor, _lod);
    }

    glm::vec4 Execute(Vertex_OUT const& _in) const override
    {
        glm::vec3 N = glm::normalize(_in.normal);
        glm::vec3 T = _in.tangent;

        //Verification
        glm::vec3 resultT = T - glm::dot(N, T) * N;
        if (glm::dot(resultT, resultT) > 1e-8f) {
            T = glm::normalize(resultT);
        }
        else {
            // Degenerated
            T = (std::abs(N.z) < 0.999f) ? glm::normalize(glm::cross(N, glm::vec3(0, 0, 1))) : glm::normalize(glm::cross(N, glm::vec3(0, 1, 0)));
        }

        glm::vec3 B = glm::cross(N, T);
        glm::mat3x3 TBN = glm::mat3x3(T, B, N);

        glm::vec3 eyeDir = Pass().eyePos - glm::vec3(_in.posModel);
        float eyeDistSq = glm::dot(eyeDir, eyeDir);
        glm::vec3 toEye = (eyeDistSq > 1e-8f) ? glm::normalize(eyeDir) : glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 viewT = glm::normalize(glm::transpose(TBN) * toEye);
        viewT.y = -viewT.y;

        glm::vec2 uv = _in.uv;
        //if (Has_displacement())
        //    uv = _in.uv - glm::vec2(viewT) / glm::max(viewT.z, 0.3f) * (Depth(_in.uv, _in.uvDdx, _in.uvDdy) * mat().heightScale);

        float rockyness = smoothstep(0.5f - mat().blendDistance, 0.5f + mat().blendDistance, _in.color.x);
        float humidity = smoothstep(0.5f - mat().blendDistance, 0.5f + mat().blendDistance, _in.color.y);
        float upWeight = smoothstep(0.6f - mat().blendDistance, 0.6f + mat().blendDistance, glm::dot(glm::vec3(0, 1, 0), _in.normal));

        float materialFactors[6];
        float rock = rockyness * (1.0f - upWeight);                                 materialFactors[0] = rock; // +rock side
        float snow = rockyness * upWeight;                                          materialFactors[1] = snow; // +rock up
        float sand = (1.0f - rockyness) * (1.0f - humidity) * upWeight;             materialFactors[2] = sand; // -rock -humid up
        float drySand = (1.0f - rockyness) * (1.0f - humidity) * (1.0f - upWeight); materialFactors[3] = drySand; // -rock -humid side
        float dirt = (1.0f - rockyness) * humidity * (1.0f - upWeight);             materialFactors[4] = dirt; // -rock +humid side
        float grass = (1.0f - rockyness) * humidity * upWeight;                     materialFactors[5] = grass; // -rock +humid up


        glm::vec4 albedo = glm::vec4(0, 0, 0, 0);
        glm::vec3 normalW = { 0, 0, 0 };
        float roughnessFactor = 0;
        if (Has_textureAtlas() && mat().useTextures)
        {
            const int textureCountPerMat = 4;
            const float tillingScale = 1 / 16.0f;
            const float atlasSideFactor = 1.0f / 5.0f;
            glm::vec2 worldUv = { _in.posModel.x * tillingScale - floor(_in.posModel.x * tillingScale), _in.posModel.z * tillingScale - floor(_in.posModel.z * tillingScale) };
            
            glm::vec2 worldTileFactor = mat().uvWorldScale * glm::vec2(tillingScale, tillingScale) * atlasSideFactor;
            glm::vec2 worldTileUvDX = _in.uvDdx * glm::vec2(worldTileFactor.x, worldTileFactor.y);
            glm::vec2 worldTileUvDY = _in.uvDdy * glm::vec2(worldTileFactor.x, worldTileFactor.y);

            // footprint of the pixel in level 0 texels, each axis scaled by its own size
            vec2 size = vec2(1024 * 5, 1024 * 5);
            vec2 dx = worldTileUvDX * size;
            vec2 dy = worldTileUvDY * size;

            // log2(sqrt(x)) = 0.5 * log2(x), avoids the two square roots
            float rho2 = glm::max(glm::dot(dx, dx), glm::dot(dy, dy));
            float lod = 0.5f * glm::log2(glm::max(rho2, 1e-12f));
            lod = glm::clamp(lod, 0.0f, (float)(11 - 1));
            uint32 l0 = (uint32)lod;
            uint32 l1 = glm::min(((uint32)lod) + 1, (uint32)11 - 1);

            int textureSize = 1024 >> l1;
            worldUv = glm::vec2(glm::clamp(worldUv.x, 2.0f / textureSize, (textureSize - 2.0f) / textureSize), glm::clamp(worldUv.y, 2.0f / textureSize, (textureSize - 2.0f) / textureSize));
            worldUv *= atlasSideFactor;

            for (int i = 0; i < 6; i++)
            {
                if (materialFactors[i] > 0.0f)
                {
                    albedo += SampleDiff(i, atlasSideFactor, worldUv, lod) * materialFactors[i];

                    glm::vec3 sampledN = glm::vec3(SampleNorm(i, atlasSideFactor, worldUv, lod)) * materialFactors[i];
                    sampledN = sampledN * 2.0f - 1.0f;
                    normalW += sampledN;

                    roughnessFactor += SampleRough(i, atlasSideFactor, worldUv, lod).x * materialFactors[i];
                }
            }
        }
        else
        {
            if (rock > 0.0f)
                albedo += glm::vec4(0.3f, 0.3f, 0.3f, 1) * rock;
            if (snow > 0.0f)
                albedo += glm::vec4(0.95f, 0.95f, 0.95f, 1) * snow;
            if (drySand > 0.0f)
                albedo += glm::vec4(0.5f, 0.5f, 0.1f, 1) * drySand;
            if (sand > 0.0f)
                albedo += glm::vec4(0.75f, 0.75f, 0.2f, 1) * sand;
            if (dirt > 0.0f)
                albedo += glm::vec4(0.2f, 0.2f, 0.05f, 1) * dirt;
            if (grass > 0.0f)
                albedo += glm::vec4(0.2f, 0.75f, 0.2f, 1) * grass;
        }

        albedo *= mat().diffuseAlbedo;
        normalW = glm::normalize(TBN * normalW);

        float metalnessFactor = mat().metalness;

        glm::vec4 diffuse = albedo * (1.0f - metalnessFactor);
        glm::vec3 F0 = glm::mix(mat().fresnelR0, glm::vec3(albedo), metalnessFactor);
        float shininess = 1.0f - roughnessFactor;

        glm::vec3 ambientFactor = Light().ambientLight * (glm::vec3(diffuse) + F0);
        if (Has_ambient())
            ambientFactor *= glm::vec3(Sample_Grad_ambient(uv, _in.uvDdx, _in.uvDdy));

        BlinnPhongData d = { diffuse, F0, shininess };
        float shadowFactor = 0.0f;
        if (Has_shadow())
            shadowFactor = ShadowCalc(_in.posLightSpace);
        glm::vec4 directLight = ComputeLighting(Light(), d, _in.posModel, normalW, toEye, shadowFactor);

        glm::vec4 litColor = glm::vec4(ambientFactor, 1.0f) + directLight;
        litColor.a = mat().diffuseAlbedo.a;

        return albedo;
    }
};

#endif
