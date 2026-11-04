#ifndef SHADERS_LIT_T_BP_HPP_DEFINED
#define SHADERS_LIT_T_BP_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../Datas/Lights/LightHelpers.hpp"

using namespace Lighting;

class LitT_BP_VS : public VertexShader
{
public:
    LitT_BP_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    inline Vertex_OUT Execute(Vertex const& _in) const override
    {
        vec4 totalPosition = vec4(0.0f);
        vec3 totalNormal = vec3(0.0f);

        bool hasBones = false;
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (_in.bone_ids[i] == -1)
                break;

            if (_in.bone_ids[i] >= MAX_BONES)
            {
                totalPosition = vec4(_in.position, 1.0f);
                break;
            }

            hasBones = true;

            vec4 localPosition = Object().finalBonesMatrices[_in.bone_ids[i]] * glm::vec4(_in.position, 1.0f);
            totalPosition += localPosition * _in.weights[i];

            vec3 localNormal = mat3(Object().finalBonesMatrices[_in.bone_ids[i]]) * _in.normal;
            totalNormal += localNormal * _in.weights[i];
        }

        if (!hasBones)
        {
            totalPosition = vec4(_in.position, 1.f);
            totalNormal = _in.normal;
        }

        Vertex_OUT v;
        v.color = _in.color;
        v.posModel = Object().model * totalPosition;
        v.posH = Pass().proj * Pass().view * v.posModel;
        v.normal = glm::mat3x3(Object().normalMatrix) * totalNormal;
        v.uv = _in.uv;

        return v;
    }
};

class LitT_BP_PS : public PixelShader
{
public:
    struct MaterialCB
    {
        glm::vec4 diffuseAlbedo;
        
        glm::vec3 fresnelR0;
        float roughness;
        
        float heightScale;
        float metalness;
    };
    
    SHADER_CB(MaterialCB, mat, CB_CUSTOM)
    SHADER_TEX_OPT(diffuse, 0)
    SHADER_TEX_OPT(arm, 1)          // r = ambient occlusion, g = roughness, b = metalness
    SHADER_TEX_OPT(displacement, 2)
    SHADER_TEX_OPT(normal, 3)
    SHADER_TEX_OPT(shadow, 4)

    LitT_BP_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    float Depth(glm::vec2 _uv, glm::vec2 _ddx, glm::vec2 _ddy) const { return 1.0f - Sample_Grad_displacement(_uv, _ddx, _ddy).x; }
    
    float ShadowCalc(glm::vec4 posLightSpace) const
    {
        vec3 projCoords = vec3(posLightSpace) / posLightSpace.w;
        projCoords = projCoords * 0.5f + 0.5f;
        float closestDepth = shadow(glm::vec2(projCoords)).r;
        float currentDepth = projCoords.z;
        
        float bias = 0.005f;
        float shadow = currentDepth - bias > closestDepth  ? 1.0f : 0.0f;
        if(projCoords.z > 1.0)
            shadow = 0.0;
        
        return shadow;
    }
    
    vec4 Execute(Vertex_OUT const& _in) const override
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
        if (Has_displacement())
            uv = _in.uv - glm::vec2(viewT) / glm::max(viewT.z, 0.3f) * (Depth(_in.uv, _in.uvDdx, _in.uvDdy) * mat().heightScale);
        
        glm::vec4 albedo;
        if (Has_diffuse())
            albedo = Sample_Grad_diffuse(uv, _in.uvDdx, _in.uvDdy) * mat().diffuseAlbedo;
        else
            albedo = _in.color * mat().diffuseAlbedo;
        
        glm::vec3 normalW;
        if (Has_normal())
        {
            glm::vec3 sampledN = glm::vec3(Sample_Grad_normal(uv, _in.uvDdx, _in.uvDdy));
            sampledN = sampledN * 2.0f - 1.0f;
            normalW = glm::normalize(TBN * sampledN);
        }
        else
            normalW = glm::normalize(_in.normal);

        float aoFactor = 1.0f;
        float roughnessFactor = mat().roughness;
        float metalnessFactor = mat().metalness;
        if (Has_arm())
        {
            glm::vec4 armSample = Sample_Grad_arm(uv, _in.uvDdx, _in.uvDdy);
            aoFactor        = armSample.r;
            roughnessFactor = armSample.g;
            metalnessFactor = armSample.b;
        }

        glm::vec4 diffuse = albedo * (1.0f - metalnessFactor);
        glm::vec3 F0 = glm::mix(mat().fresnelR0, glm::vec3(albedo), metalnessFactor);
        float shininess = 1.0f - roughnessFactor;

        glm::vec3 ambientFactor = Light().ambientLight * (glm::vec3(diffuse) + F0) * aoFactor;
        
        BlinnPhongData d = { diffuse, F0, shininess};
        float shadowFactor = 0.0f;
        if (Has_shadow())
            shadowFactor = ShadowCalc(_in.posLightSpace);
        glm::vec4 directLight = ComputeLighting(Light(), d, _in.posModel, normalW, toEye, shadowFactor);

        vec4 litColor = glm::vec4(ambientFactor, 1.0f) + directLight;
        litColor.a = mat().diffuseAlbedo.a;
        
        return litColor;
    }
};

#endif
