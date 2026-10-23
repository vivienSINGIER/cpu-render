#ifndef SHADERS_UNLIT_C_HPP_DEFINED
#define SHADERS_UNLIT_C_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

class UnlitC_VS : public VertexShader
{
public:
    UnlitC_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}  
    
    Vertex_OUT Execute(Vertex const& _in) const override
    {
        Vertex_OUT v;
        v.color = _in.color;
        v.positionModel = Object().model * glm::vec4({ _in.position, 1 });
        v.positionH = Pass().proj * Pass().view * v.positionModel;
        v.normal = glm::mat3x3(Object().model) * _in.normal;
        v.uv = _in.uv;
        return v;
    }
};

class UnlitC_PS : public PixelShader
{
public:
    struct UnlitColor
    {
        Color color;
    };
    
    SHADER_CB(UnlitColor, mat, CB_CUSTOM)

    UnlitC_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    Color Execute(Vertex_OUT const& _in) const override
    {
        return mat().color;
    }
};

#endif
