#ifndef SHADERS_UNLIT_T_HPP_DEFINED
#define SHADERS_UNLIT_T_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

class UnlitT_VS : public VertexShader
{
public:
    UnlitT_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}  
    
    Vertex_OUT Execute(Vertex const& _in) const override
    {
        Vertex_OUT v;
        v.color = _in.color;
        v.posModel = Object().model * glm::vec4({ _in.position, 1 });
        v.posH = Pass().proj * Pass().view * v.posModel;
        v.normal = glm::mat3x3(Object().model) * _in.normal;
        v.uv = _in.uv;
        return v;
    }
};

class UnlitT_PS : public PixelShader
{
public:
    SHADER_TEX(albedo, 0)

    UnlitT_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    vec4 Execute(Vertex_OUT const& _in) const override
    {
        return Sample_Grad_albedo(_in.uv, _in.uvDdx, _in.uvDdy);
    }
};

#endif
