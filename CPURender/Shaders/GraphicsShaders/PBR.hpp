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
    
    SHADER_TEX_OPT(diffuse, 3)
    SHADER_TEX_OPT(normal, 4)
    SHADER_TEX_OPT(arm, 5)

    PBR_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}
    
    vec4 Execute(Vertex_OUT const& _in) const override
    {
        vec3 albedo = mat().albedo;
        vec3 N = glm::normalize(_in.normal);
        vec3 normalW = N;
        float ao = mat().ambientOcc;
        float roughness = mat().roughness;
        float metalness = mat().metalness;
        
        if (Has_diffuse())
            albedo = diffuse(_in.uv);
        if (Has_normal())
        {
            vec3 T = glm::normalize(_in.tangent);                                                                                                                                                                                         
            T = glm::normalize(T - glm::dot(N, T) * N);   // Gram-Schmidt re-orthogonalize                                                                                                                                                
            vec3 B = glm::cross(N, T);                                                                                                                                                                                                    
            mat3x3 TBN = mat3x3(T, B, N);                                                                                                                                                                                                 
                                                                                                                                                                                                                                    
            vec3 sampledN = vec3(normal(_in.uv)) * 2.0f - 1.0f;  // [0,1] -> [-1,1]                                                                                                                                                       
            normalW = glm::normalize(TBN * sampledN);   
        }
        if (Has_arm())
        {
            vec3 armS = arm(_in.uv);
            ao = armS.r;
            roughness = armS.g;
            roughness = glm::clamp(roughness, 0.05f, 1.0f);
            metalness = armS.b;
        }
        
        vec3 V = normalize(Pass().eyePos - vec3(_in.posModel));
        
        vec3 R = reflect(-V, normalW);
        
        vec3 F0 = vec3(0.04f);
        F0 = glm::mix(F0, albedo, metalness);
        
        vec3 F = fresnelSchlickRoughness(max(dot(normalW, V), 0.0f), F0, roughness);
        
        vec3 kS = F;
        vec3 kD = 1.0f - kS;
        kD *= 1.0f - metalness;
        
        vec3 irradience = vec3(Sample_Cube_irradience(normalW));
        vec3 diffuse = irradience * albedo;
        
        const float MAX_REFLECTION_LOD = 4.0f;
        vec3 prefiltered = Sample_Cube_Lod_prefilter(R, roughness * MAX_REFLECTION_LOD);
        vec2 envBRDF = brdfLUT(vec2(max(dot(normalW, V), 0.0f), roughness));
        vec3 specular = prefiltered * (F0 * envBRDF.x + envBRDF.y);
        
        vec3 ambient = (kD * diffuse + specular) * ao;
        
        PBRData d = { albedo, metalness, F0, roughness, ao };
        vec3 Lo = ComputePBR(Light(), d, _in.posModel, normalW, V);
        
        vec3 color = ambient + Lo;
        
        return vec4(color, 1.0f);
    }
};

#endif
