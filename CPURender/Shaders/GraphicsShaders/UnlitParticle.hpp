#ifndef SHADERS_UNLIT_PARTICLE_HPP_DEFINED
#define SHADERS_UNLIT_PARTICLE_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"
#include "../../Datas/Particle.h"

class UnlitParticle_VS : public VertexShader
{
    SHADER_UAV_BUFF(UavParticle, data, 0)

public:
    UnlitParticle_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    Vertex_OUT Execute(Vertex const& _in) const override
    {
        size_t quad = _in.id / 6;
        size_t corner = _in.id % 6; //0, 2, 1,    3, 1, 2 indices

        const UavParticle& p = Load_data(quad);

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
        glm::vec4 color;
    };

    SHADER_CB(ParticleColor, pcolor, CB_CUSTOM)

    UnlitParticle_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    glm::vec4 Execute(Vertex_OUT const& _in) const override
    {
        return pcolor().color;

        //glm::vec4 objColor = Particle().color;
        //return Color(matColor.a * objColor.a, matColor.r * objColor.r, matColor.g * objColor.g, matColor.b * objColor.b);
    }
};

#endif

