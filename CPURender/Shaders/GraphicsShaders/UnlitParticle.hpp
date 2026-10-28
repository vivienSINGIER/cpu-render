#ifndef SHADERS_UNLIT_PARTICLE_HPP_DEFINED
#define SHADERS_UNLIT_PARTICLE_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

class UnlitParticle_VS : public VertexShader
{
public:
    UnlitParticle_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

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

class UnlitParticle_PS : public PixelShader
{
public:
    struct ParticleColor
    {
        Color color;
    };

    SHADER_CB(ParticleColor, pcolor, CB_CUSTOM)

    UnlitParticle_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    Color Execute(Vertex_OUT const& _in) const override
    {
        Color color = pcolor().color;
        return color;

        //glm::vec4 objColor = Particle().color;
        //return Color(matColor.a * objColor.a, matColor.r * objColor.r, matColor.g * objColor.g, matColor.b * objColor.b);
    }
};

#endif

