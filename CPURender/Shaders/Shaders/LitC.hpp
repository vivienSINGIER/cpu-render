#ifndef SHADERS_LIT_C_HPP_DEFINED
#define SHADERS_LIT_C_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"
#include <iostream>

class LitC_VS : public VertexShader
{
public:
    LitC_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    inline Vertex_OUT Execute(ShaderResources const& _r, Vertex const& _in) const override
    {
        PerPassCB const& pCB = CB<PerPassCB>(_r, 0);
        PerObjectCB const& oCB = CB<PerObjectCB>(_r, 1);

        Vertex_OUT v;
        v.normal = _in.normal;
        v.color = _in.color;
        v.positionH = glm::vec4({ _in.position, 1 });
        v.positionH = pCB.proj * pCB.view * oCB.model * v.positionH;
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

    LitC_PS(GraphicsContext* _ctx) : PixelShader(_ctx) { AddConstantBuffer<MaterialData>(CB_CUSTOM); }

    inline Color Execute(ShaderResources const& _r, Vertex_OUT const& _in) const override
    {
        PerPassCB const& pCB = CB<PerPassCB>(_r, 0);
        MaterialData const& cb = CB<MaterialData>(_r, 2);

        float ambientFactors[3] = { 0.1f, 0.1f, 0.15f };
        Color ambientColor = { 255,
            (uint8_t)(cb.color.r * ambientFactors[0]) ,
            (uint8_t)(cb.color.g * ambientFactors[1]) ,
            (uint8_t)(cb.color.b * ambientFactors[2]) };

        float dot = glm::dot(pCB.lightDirection, _in.normal);
        return cb.color * -((dot + 1) / 2.0f) * pCB.lightIntensity + ambientColor;
    }
};

#endif
