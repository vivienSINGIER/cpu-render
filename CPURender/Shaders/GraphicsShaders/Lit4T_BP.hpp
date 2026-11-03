#ifndef SHADERS_LIT_4T_BP_HPP_DEFINED
#define SHADERS_LIT_4T_BP_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../Datas/Lights/LightHelpers.hpp"

using namespace Lighting;

class Lit4T_BP_VS : public VertexShader
{
public:
    Lit4T_BP_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    SHADER_TEX_OPT(texWeight, 4)

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

class Lit4T_BP_PS : public PixelShader
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
    };

    SHADER_CB(MaterialCB, mat, CB_CUSTOM)
        SHADER_TEX_OPT(diffuseAtlas, 0)

        SHADER_TEX_OPT(roughness, 5)
        SHADER_TEX_OPT(displacement, 6)
        SHADER_TEX_OPT(normal, 7)
        SHADER_TEX_OPT(ambient, 8)
        SHADER_TEX_OPT(metal, 9)
        SHADER_TEX_OPT(shadow, 10)

        Lit4T_BP_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    float Depth(glm::vec2 _uv, glm::vec2 _ddx, glm::vec2 _ddy) const { return 1.0f - Sample_Grad_displacement(_uv, _ddx, _ddy).x; }

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
        if (Has_displacement())
            uv = _in.uv - glm::vec2(viewT) / glm::max(viewT.z, 0.3f) * (Depth(_in.uv, _in.uvDdx, _in.uvDdy) * mat().heightScale);

        const float blendDistance = 0.05f;
        float rock = smoothstep(0.5f - blendDistance, 0.5f + blendDistance, _in.color.x);
        float humid = smoothstep(0.5f - blendDistance, 0.5f + blendDistance, _in.color.y);
        glm::vec4 weight = glm::vec4(
            (1.0f - rock) * (1.0f - humid),             // -rock, -humid
            rock * (1.0f - humid),                      // +rock, -humid
            (1.0f - rock) * humid,                      // -rock, +humid
            rock * humid                                // +rock, +humid
        );

        glm::vec4 albedo = glm::vec4(0, 0, 0, 0);
        const float tillingScale = 1 / 16.0f;
        glm::vec2 worldUv = { _in.posModel.x * tillingScale - floor(_in.posModel.x * tillingScale), _in.posModel.z * tillingScale - floor(_in.posModel.z * tillingScale)};
        worldUv *= 0.5f;

        glm::vec2 factor = mat().uvWorldScale * glm::vec2(tillingScale, tillingScale) / 2.0f;

        if (Has_diffuseAtlas())
        {
            if (weight.x > 0.0f)
                albedo += Sample_Grad_diffuseAtlas(worldUv + glm::vec2(0.0f, 0.0f), _in.uvDdx * factor.x, _in.uvDdy * factor.y) * weight.x;
            if (weight.y > 0.0f)
                albedo += Sample_Grad_diffuseAtlas(worldUv + glm::vec2(0.5f, 0.0f), _in.uvDdx * factor.x, _in.uvDdy * factor.y) * weight.y;
            if (weight.z > 0.0f)
                albedo += Sample_Grad_diffuseAtlas(worldUv + glm::vec2(0.0f, 0.5f), _in.uvDdx * factor.x, _in.uvDdy * factor.y) * weight.z;
            if (weight.w > 0.0f)
                albedo += Sample_Grad_diffuseAtlas(worldUv + glm::vec2(0.5f, 0.5f), _in.uvDdx * factor.x, _in.uvDdy * factor.y) * weight.w;
        }

        albedo *= mat().diffuseAlbedo;

        glm::vec3 normalW;
        if (Has_normal())
        {
            glm::vec3 sampledN = glm::vec3(Sample_Grad_normal(uv, _in.uvDdx, _in.uvDdy));
            sampledN = sampledN * 2.0f - 1.0f;
            normalW = glm::normalize(TBN * sampledN);
        }
        else
            normalW = glm::normalize(_in.normal);

        float roughnessFactor;
        if (Has_roughness())
            roughnessFactor = Sample_Grad_roughness(uv, _in.uvDdx, _in.uvDdy).x;
        else
            roughnessFactor = mat().roughness;

        float metalnessFactor;
        if (Has_metal())
            metalnessFactor = Sample_Grad_metal(uv, _in.uvDdx, _in.uvDdy).b;
        else
            metalnessFactor = mat().metalness;

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

        return litColor;
    }
};

#endif
