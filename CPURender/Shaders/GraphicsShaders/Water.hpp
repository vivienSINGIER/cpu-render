#ifndef SHADERS_GRAPHICS_WATER_HPP_DEFINED
#define SHADERS_GRAPHICS_WATER_HPP_DEFINED

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../main.h"

class Water_VS : public VertexShader
{
public:
    Water_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}  
    
    Vertex_OUT Execute(Vertex const& _in) const override
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

class Water_PS : public PixelShader
{
public:
    Water_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}
    
    SHADER_TEX(depth, 0)
    SHADER_TEX(screen, 1)
    
    static inline float MAX_DEPTH = 2.0f;
    static inline float WATER_LEVEL = 0.0f;
    static inline float FADE_START_RANGE = 0.5f;
    static inline vec3 WATER_COLOR = vec3(0.0, 0.05, 0.1);
    
    vec4 Execute(Vertex_OUT const& _in) const override
    {
        float sDepth = depth(_in.screenUV).r;
        vec3 ndc = vec3(_in.screenUV, sDepth) * 2.0f - 1.0f;
        
        vec4 world = Pass().invViewProj * vec4(ndc, 1.0f);
        vec3 pos = vec3(world) / world.w; 
        
        float surface_y = WATER_LEVEL;
        float thickness = surface_y - pos.y;
        thickness = glm::max(0.0f, thickness);
        
        vec3 screenColor = screen(_in.screenUV);
        float fade_range = max(0.001f, MAX_DEPTH - FADE_START_RANGE);
        float depth_ratio = clamp((thickness - FADE_START_RANGE) / fade_range, 0.0f, 1.0f);
        
        float grayscale = clamp(thickness / MAX_DEPTH, 0.0f, 1.0f);
        
        return vec4(mix(screenColor, WATER_COLOR, depth_ratio), 1.0f);
    }
};

#endif