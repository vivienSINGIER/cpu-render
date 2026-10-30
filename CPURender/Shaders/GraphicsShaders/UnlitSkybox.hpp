#pragma once
#ifndef SHADERS_UNLIT_SKYBOX_HPP_DEFINED
#define SHADERS_UNLIT_SKYBOX_HPP_DEFINED

#include <algorithm>
#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

class UnlitSkybox_VS : public VertexShader
{
public:
    UnlitSkybox_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    Vertex_OUT Execute(Vertex const& _in) const override
    {
        Vertex_OUT v;
        v.color = _in.color;
        v.posModel = Object().model * glm::vec4({ _in.position, 1 });
        v.posH = Pass().proj * glm::mat4(glm::mat3(Pass().view)) * v.posModel;
        v.normal = glm::mat3x3(Object().model) * _in.normal;
        v.uv = _in.uv;
        return v;
    }
};

class UnlitSkybox_PS : public PixelShader
{
public:
    SHADER_TEX(albedo, 0)

    UnlitSkybox_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    vec4 Execute(Vertex_OUT const& _in) const override
    {
        vec3 dir = glm::normalize(glm::vec3(_in.posModel));
        return Sample_Cube_Lod_albedo(dir, 2.0f);
    }
};

#endif