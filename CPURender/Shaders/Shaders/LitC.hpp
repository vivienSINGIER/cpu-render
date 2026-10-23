#ifndef SHADERS_LIT_C_HPP_DEFINED
#define SHADERS_LIT_C_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"
#include <iostream>

#include "../../Datas/Lights/LightData.hpp"

class LitC_VS : public VertexShader
{
public:
    LitC_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    inline Vertex_OUT Execute(Vertex const& _in) const override
    {
        Vertex_OUT v;
        v.normal = _in.normal;
        v.color = _in.color;
        v.positionH = glm::vec4({ _in.position, 1 });
        v.positionH = Pass().proj * Pass().view * Object().model * v.positionH;
        v.wPreDiv = 1.0f;
        return v;
    }
};

class LitC_PS : public PixelShader
{
public:
    struct MaterialData
    {
        Color color;
    };

    SHADER_CB(MaterialData, mat, CB_CUSTOM)

    LitC_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    inline Color Execute(Vertex_OUT const& _in) const override
    {
        float ambientFactors[3] = { 0.1f, 0.1f, 0.15f };
        Color ambientColor = { 255,
            (uint8_t)((float)mat().color.r * ambientFactors[0]) ,
            (uint8_t)((float)mat().color.g * ambientFactors[1]) ,
            (uint8_t)((float)mat().color.b * ambientFactors[2]) };

        float dot = glm::dot(Pass().lightDirection, _in.normal);
        return mat().color * -((dot + 1) / 2.0f) * Pass().lightIntensity + ambientColor;
    }
};

#endif
