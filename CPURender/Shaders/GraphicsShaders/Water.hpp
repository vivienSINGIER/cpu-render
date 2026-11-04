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
        p.y += hs.x;
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
        
        vec3 foamColor = vec3(0.9f, 0.95f, 1.0f);
        float crestFoamStart = 0.2f;
        
        float crestFoamWidth = 0.25f;
        float shoreFoamWidth = 0.8f;
        
        float causticScale = 0.5f;
        float causticStrength = 1.5f;
        float causticDepthFade = 0.6f;
        
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
        
        vec3 toEye = Pass().eyePos - surfacePos;
        float dist  = glm::length(toEye);
        vec3  V     = toEye / dist;
        
        float pixelAngle = 2.0f * glm::tan(0.25f * glm::pi<float>() / 2.0f) / (float)HEIGHT;
        float eps = glm::max(dist * pixelAngle * 2.0f, 0.05f);   // ~2 pixels in world units

        float worldPerUV = glm::length(vec3(Object().model[0]));   // = scaleW (plane is 1 unit wide in UV)
        vec2 ddx = _in.uvDdx * worldPerUV;
        vec2 ddy = _in.uvDdy * worldPerUV;

        vec3 hs = WaveSim::SeaHeight(*this, 2, vec2(surfacePos.x, surfacePos.z), Pass().totalTime, 5, ddx, ddy, waves());
        float slope = glm::length(vec2(hs.y, hs.z));
        vec3 N = glm::normalize(vec3(-hs.y, 1.0f, -hs.z));

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
        
        // CAUSTICS
        float k = 1.0f / (8.0f * 0.8f);
        vec2 causticPos = vec2( pos.x, pos.z ) * k;
        vec2 rCausticPos = { -causticPos.y, causticPos.x };
        vec3 causticN1 = Sample_Grad_seaBase(causticPos * mat().causticScale + vec2(Pass().totalTime) * 0.01f, ddx * k, ddy * k);
        vec3 causticN2 = Sample_Grad_seaBase(rCausticPos * mat().causticScale * 1.3f - vec2(Pass().totalTime) * 0.03f, ddx * k, ddy * k);
        
        float caustic = min(causticN1.x, causticN2.x);
        caustic *= mat().causticStrength 
                * exp(-verticalLength * mat().causticDepthFade)         // Weaker in deep water
                * smoothstep(0.0f, 0.1f, verticalLength);     // None above the waterline
        caustic = pow(caustic, 4); // Sharpen
        // return vec4(vec3(caustic), 1);
        
        vec3 screenColor = screen(_in.screenUV);
        screenColor *= 1.0f + caustic;
        vec3 col = mix(screenColor, mat().shallowColor, mat().shallowOpacity);
        
        // DEPTH FOG
        float fog = 1.0f - glm::exp(-mat().fogDensity * viewDist);
        col = mix(col, mat().waterFogColor, fog);
        
        // FOAM
        float crestHeight = _in.posModel.y - mat().waterLevel;
        crestHeight /= 2.5f * waves().height;
        
        float crestFoam = smoothstep(mat().crestFoamStart, mat().crestFoamStart + mat().crestFoamWidth, crestHeight);
        crestFoam *= smoothstep(0.3f, 0.8f, slope);
        
        float shoreFoam = 0.0f;
        if (sDepth < 1.0f)
        {
            shoreFoam = 1.0f - smoothstep(0.0f, mat().shoreFoamWidth, verticalLength);
            shoreFoam *= 0.5f + 0.5f * sin(verticalLength * 15 - Pass().totalTime * 1.0f);   
        }
        
        vec2 foamUv1 = vec2(surfacePos.x, surfacePos.z) * 0.3f + Pass().totalTime * 0.01f;
        vec2 foamUv2 = vec2(surfacePos.x, surfacePos.z) * 0.8f + Pass().totalTime * 0.01f;
        vec3 foamNoise1 = Sample_Grad_seaBase(foamUv1, ddx * 0.3f, ddy * 0.3f);
        vec3 foamNoise2 = Sample_Grad_seaBase(foamUv2, ddx * 0.8f, ddy * 0.8f);
        
        float n = 0.65f * foamNoise1.x + 0.35f * foamNoise2.x;
        float soft = 0.05f; // arbritraire
        float foamMask = max(crestFoam, shoreFoam);
        float foam = smoothstep(n - soft, n + soft, foamMask);
        // return vec4(vec3(foam), 1.0f);
        
        // ACCUMULATION
        col = mix(col, refl, F) + spec;
        col = mix(col, mat().foamColor, foam);
        
        return vec4(col, 1.0f);
    }
};

#endif