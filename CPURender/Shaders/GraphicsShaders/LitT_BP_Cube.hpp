#ifndef SHADERS_LIT_T_BP_CUBE_HPP_DEFINED
#define SHADERS_LIT_T_BP_CUBE_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../Datas/Lights/LightHelpers.hpp"

using namespace Lighting;

class LitT_BP_Cube_VS : public VertexShader
{
public:
    LitT_BP_Cube_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    inline Vertex_OUT Execute(Vertex const& _in) const override
    {
        Vertex_OUT v;
        v.color = _in.color;
        v.posModel = Object().model * glm::vec4({ _in.position, 1 });
        v.posH = v.posModel * Pass().viewProj;
        v.posLightSpace = v.posModel * Pass().lightSpace;
        v.normal = glm::mat3x3(Object().normalMatrix) * _in.normal;
        v.tangent = glm::mat3x3(Object().normalMatrix) * _in.tangent;
        v.uv = _in.uv;
        return v;
    }
};

class LitT_BP_Cube_PS : public PixelShader
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
        SHADER_TEX_OPT(roughness, 1)
        SHADER_TEX_OPT(displacement, 2)
        SHADER_TEX_OPT(normal, 3)
        SHADER_TEX_OPT(ambient, 4)
        SHADER_TEX_OPT(metal, 5)
        SHADER_TEX_OPT(shadow, 6)
        SHADER_TEX_OPT(cubemap, 7)

        LitT_BP_Cube_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    inline float Depth(glm::vec2 _uv) const { return 1.0f - displacement(_uv).x; }

    inline float ShadowCalc(glm::vec4 posLightSpace) const
    {
        glm::vec3 projCoords = glm::vec3(posLightSpace) / posLightSpace.w;
        projCoords = projCoords * 0.5f + 0.5f;
        float closestDepth = shadow(glm::vec2(projCoords)).r;
        float currentDepth = projCoords.z;

        float bias = 0.005f;
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
        if (projCoords.z > 1.0)
            shadow = 0.0;

        return shadow;
    }

    inline vec4 Execute(Vertex_OUT const& _in) const override
    {
        glm::vec3 T = glm::normalize(_in.tangent);
        glm::vec3 N = glm::normalize(_in.normal);
        T = glm::normalize(T - glm::dot(N, T) * N);
        glm::vec3 B = glm::cross(N, T);
        glm::mat3x3 TBN = glm::mat3x3(T, B, N);

        glm::vec3 toEye = glm::normalize(Pass().eyePos - glm::vec3(_in.posModel));
        glm::vec3 viewT = glm::transpose(TBN) * toEye;
        viewT.y = -viewT.y;

        glm::vec2 uv = _in.uv;
        if (Has_displacement())
            uv = _in.uv - glm::vec2(viewT) / glm::max(viewT.z, 0.3f) * (Depth(_in.uv) * mat().heightScale);

        glm::vec4 albedo;
        if (Has_diffuse())
            albedo = diffuse(uv) * mat().diffuseAlbedo;
        else
            albedo = _in.color * mat().diffuseAlbedo;

        glm::vec3 normalW;
        if (Has_normal())
        {
            glm::vec3 sampledN = glm::vec3(normal(uv));
            sampledN = sampledN * 2.0f - 1.0f;
            normalW = glm::normalize(TBN * sampledN);
        }
        else
            normalW = glm::normalize(_in.normal);

        float roughnessFactor;
        if (Has_roughness())
            roughnessFactor = roughness(uv).x;
        else
            roughnessFactor = mat().roughness;

        float metalnessFactor;
        if (Has_metal())
            metalnessFactor = metal(uv).b;
        else
            metalnessFactor = mat().metalness;

        glm::vec4 diffuse = albedo * (1.0f - metalnessFactor);
        glm::vec3 F0 = glm::mix(mat().fresnelR0, glm::vec3(albedo), metalnessFactor);
        float shininess = 1.0f - roughnessFactor;

        glm::vec3 ambientFactor = Light().ambientLight * (glm::vec3(diffuse) + F0);
        if (Has_cubemap())
            ambientFactor *= glm::vec3(Sample_Cube_cubemap(glm::reflect(-toEye, N)));
        if (Has_ambient())
            ambientFactor *= glm::vec3(ambient(uv));

        BlinnPhongData d = { diffuse, F0, shininess };
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
