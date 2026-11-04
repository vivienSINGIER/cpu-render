#ifndef SHADERS_GRAPHICS_WATER_HPP_DEFINED
#define SHADERS_GRAPHICS_WATER_HPP_DEFINED

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../main.h"
#include "../../Maths/WaveSim.hpp"

// CPU friendly SEASCAPE shader implementation

class Water_VS : public VertexShader
{
public:
    Water_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}  
    
    SHADER_CB(WaveSim::SeaParams, waves, CB_CUSTOM + 1)
    SHADER_TEX(seaBase, 2)
    
    Vertex_OUT Execute(Vertex const& _in) const override
    {
        Vertex_OUT v;
        
        vec3 p = vec3(Object().model * vec4(_in.position, 1.0f));
        vec3 hs = WaveSim::SeaHeight(*this, 2, { p.x, p.z }, Pass().totalTime, 3, 0.0f, waves());
        p.y += hs.y;
        v.posModel = vec4(p, 1.0f);
        v.normal = glm::normalize(vec3(-hs.y, 1.0f, -hs.z));
        v.posH = v.posModel * Pass().viewProj;
        v.posLightSpace = v.posModel * Pass().lightSpace;
        v.uv = _in.uv;
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
    SHADER_CB(WaveSim::SeaParams, waves, CB_CUSTOM + 1)
    
    SHADER_TEX(depth, 0)
    SHADER_TEX(screen, 1)
    SHADER_TEX(seaBase, 2)
    
    SHADER_TEX(envCubeMap, 3)
    
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
        
        vec3 toEye = Pass().eyePos - surfacePos;
        float dist  = glm::length(toEye);
        vec3  V     = toEye / dist;
        
        float pixelAngle = 2.0f * glm::tan(0.25 * glm::pi<float>() / 2.0f) / HEIGHT;
        float eps = glm::max(dist * pixelAngle * 2.0f, 0.05f);   // ~2 pixels in world units
        
        float h0 = SeaHeight(*this, 2, vec2(surfacePos.x,       surfacePos.z      ), Pass().totalTime, 5, _in.uvDdx, _in.uvDdy, waves()).x;
        float hx = SeaHeight(*this, 2, vec2(surfacePos.x + eps, surfacePos.z      ), Pass().totalTime, 5, _in.uvDdx, _in.uvDdy, waves()).x;
        float hz = SeaHeight(*this, 2, vec2(surfacePos.x,       surfacePos.z + eps), Pass().totalTime, 5, _in.uvDdx, _in.uvDdy, waves()).x;

        vec3 N = glm::normalize(vec3(-(hx - h0) / eps, 1.0f, -(hz - h0) / eps));
        float slope    = glm::length(vec3(-(hx - h0) / eps, 1.0f, -(hz - h0) / eps));
        
        vec3 R = glm::reflect(-V, N);
        R.y = glm::abs(R.y);
        R = glm::normalize(R);
        
        float rough    = glm::clamp(dist / 400.0f + slope * 0.5f, 0.0f, 1.0f);
        float cubeLod  = 5.0f * rough;
        vec3 refl = Sample_Cube_Lod_envCubeMap(R, cubeLod);
        
        float NdotV = max(dot(N, V), 1e-3f);
        vec3 F0 = Lighting::g_fresnelR0Templates[Lighting::WATER];
        vec3 F = Lighting::fresnelSchlick(NdotV, F0);
        
        Lighting::BlinnPhongData bp = {
            vec4(0.0f),
            F0,
            0.95f
        };
        
        vec3 spec = vec3(ComputeLighting(Light(), bp, surfacePos, N, V, 0.0f));
        col = mix(col, refl, F) + spec;
        
        return vec4(col, 1.0f);
    }
};

#endif