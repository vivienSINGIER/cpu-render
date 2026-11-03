#ifndef SHADERS_GRAPHICS_WATER_HPP_DEFINED
#define SHADERS_GRAPHICS_WATER_HPP_DEFINED

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../main.h"

class Water_VS : public VertexShader
{
public:
    Water_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}  
    
    struct WaveCB
    {
        vec4 waves[4];  // xy -> direction | z -> steepness | w = wavelength
        uint32 waveCount = 0;
    };
    
    SHADER_CB(WaveCB, waves, CB_CUSTOM + 1)
    
    Vertex_OUT Execute(Vertex const& _in) const override
    {
        // Gerstner waves
        
        vec3 p = vec3(Object().model * vec4(_in.position, 1.0f));
        vec3 grid = p;
        vec3 tangent = vec3(1, 0, 0);
        vec3 binormal = vec3(0, 0, 1);
        float t = Pass().totalTime;
        
        for (uint32 i = 0; i < waves().waveCount; i++)
        {
            vec4 w = waves().waves[i];
            vec2 d = normalize(vec2(w.x, w.y));
            float s = w.z;
            float k = 2.0f * glm::pi<float>() / w.w; // wave number
            float c = glm::sqrt(9.8f / k); // phase speed
            float f = k * (glm::dot(d, vec2(grid.x, grid.z)) - c * t);
            float a = s / k;
            
            float sf = glm::sin(f);
            float cf = glm::cos(f);
            p.x += d.x * a * cf;
            p.y += a * sf;
            p.z += d.y * a * cf;
            
            tangent  += vec3(-d.x * d.x * s * sf, d.x * s * cf, -d.x * d.y * s * sf);
            binormal += vec3(-d.x * d.y * s * sf, d.y * s * cf, -d.y * d.y * s * sf);
        }
        
        Vertex_OUT v;
        v.posModel = vec4(p, 1.0f);
        v.posH = v.posModel * Pass().viewProj;
        v.normal = glm::normalize(glm::cross(binormal, tangent));
        v.tangent = glm::normalize(tangent);
        v.uv = _in.uv;
        v.color = _in.color;
        return v;
    }
    
};

class Water_PS : public PixelShader
{
public:
    Water_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}
    
    struct WaterMat
    {
        vec3 waterFogColor = vec3(0.0, 0.05, 0.1);
        float maxDepth = 3.0f;
        
        vec3 shallowColor = vec3(0.1f, 0.4f, 0.5f);
        float waterLevel = 0.0f;
        
        float fogDensity = 0.8f;
        float shallowOpacity = 0.3f;
        float fadeStartDepth = 0.0f;
    };
    
    SHADER_CB(WaterMat, mat, CB_CUSTOM)
    
    SHADER_TEX(depth, 0)
    SHADER_TEX(screen, 1)
    
    vec4 Execute(Vertex_OUT const& _in) const override
    {
        float sDepth = depth(_in.screenUV).r;
        vec3 ndc = vec3(_in.screenUV, sDepth) * 2.0f - 1.0f;
        vec4 world = Pass().invViewProj * vec4(ndc, 1.0f);
        vec3 pos = vec3(world) / world.w; 
        
        vec3 surfacePos = vec3(_in.posModel);
        float viewDist = glm::length(pos - surfacePos);
        float verticalLength = glm::max(surfacePos.y - pos.y, 0.0f);
        
        vec3 screenColor = screen(_in.screenUV);
        vec3 col = mix(screenColor, mat().shallowColor, mat().shallowOpacity);
        
        float fog = 1.0f - glm::exp(-mat().fogDensity * viewDist);
        col = mix(col, mat().waterFogColor, fog);
        
        vec3 N = glm::normalize(_in.normal);
        vec3 toEye = glm::normalize(Pass().eyePos - surfacePos);
        
        Lighting::BlinnPhongData bp = {                                                                                                                                                                                              
            vec4(0.0f),                                                                                                                                               
            Lighting::g_fresnelR0Templates[Lighting::WATER],                                                                                                                                                          
            0.95f                                                                                                                                            
        };  
        
        vec3 spec = vec3(ComputeLighting(Light(), bp, surfacePos, N, toEye, 0.0f));
        col += spec;
        
        return vec4(col, 1.0f);
    }
};

#endif