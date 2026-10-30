#ifndef SHADERS_PBR_HPP_DEFINED
#define SHADERS_PBR_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../main.h"

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../Datas/Lights/LightHelpers.hpp"
#include "../../Datas/Lights/PBRHelpers.hpp"

class PBR_VS : public VertexShader
{
public:
    PBR_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    inline Vertex_OUT Execute(Vertex const& _in) const override
    {
        Vertex_OUT v;
        v.color = _in.color;
        v.posModel = Object().model * vec4({ _in.position, 1 });
        v.posH = v.posModel * Pass().viewProj;
        v.posLightSpace = v.posModel * Pass().lightSpace;
        v.normal = mat3x3(Object().normalMatrix) * _in.normal;
        v.tangent = mat3x3(Object().normalMatrix) * _in.tangent;
        v.uv = _in.uv;
        return v;
    }
};

class PBR_PS : public PixelShader
{
public:
    struct MaterialCB
    {
        vec3 albedo;
        float metalness;
        
        float roughness;
        float ambientOcc;
    };
    
    SHADER_CB(MaterialCB, mat, CB_CUSTOM)
    SHADER_TEX(irradience, 0)
    SHADER_TEX(prefilter, 1)
    SHADER_TEX(brdfLUT, 2)

    PBR_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}
    
    vec4 Execute(Vertex_OUT const& _in) const override
    {
        vec3 normal = glm::normalize(_in.normal);
        vec3 V = normalize(Pass().eyePos - vec3(_in.posModel));
        
        vec3 R = reflect(-V, normal);
        
        vec3 F0 = vec3(0.04f);
        F0 = glm::mix(F0, mat().albedo, mat().metalness);
        
        vec3 F = fresnelSchlickRoughness(max(dot(normal, V), 0.0f), F0, mat().roughness);
        
        vec3 kS = F;
        vec3 kD = 1.0f - kS;
        kD *= 1.0f - mat().metalness;
        
        vec3 irradience = vec3(Sample_Cube_irradience(normal));
        vec3 diffuse = irradience * mat().albedo;
        
        const float MAX_REFLECTION_LOD = 4.0f;
        vec3 prefiltered = Sample_Cube_Lod_prefilter(R, mat().roughness * MAX_REFLECTION_LOD);
        vec2 envBRDF = brdfLUT(vec2(max(dot(normal, V), 0.0f), mat().roughness));
        vec3 specular = prefiltered * (F0 * envBRDF.x + envBRDF.y);
        
        vec3 ambient = (kD * diffuse + specular) * mat().ambientOcc;
        
        PBRData d = { mat().albedo, mat().metalness, F0, mat().roughness, mat().ambientOcc };
        vec3 Lo = ComputePBR(Light(), d, _in.posModel, normal, V);
        
        vec3 color = ambient + Lo;
        
        return vec4(color, 1.0f);
    }
};

#endif
