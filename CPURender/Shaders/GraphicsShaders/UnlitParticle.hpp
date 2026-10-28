#ifndef SHADERS_UNLIT_PARTICLE_HPP_DEFINED
#define SHADERS_UNLIT_PARTICLE_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"
#include "../../Datas/Particle.h"

#include <iostream>

class UnlitParticle_VS : public VertexShader
{
    SHADER_UAV_BUFF(UavParticle, data, 0)

public:
    UnlitParticle_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    Vertex_OUT Execute(Vertex const& _in) const override
    {
        int quad = (int)_in.id / 6;
        int corner = (int)_in.id % 6; //0, 2, 1,    3, 1, 2 indices

        const UavParticle& p = Load_data(quad);

        if (p.lifeTime <= 0.f)
        {
            Vertex_OUT error;
            error.color = { -1, -1, -1, -1 };
            return error;
        }

        glm::vec3 offset = {};

        switch (corner)
        {
        case 0:
            offset = { -0.5f, -0.5f, 0.f };
            break;
        case 3:
            offset = { 0.5f, 0.5f, 0.f };
            break;
        case 1:
        case 5:
            offset = { 0.5f, -0.5f, 0.f };
            break;
        case 2:
        case 4:
            offset = { -0.5f, 0.5f, 0.f };
            break;
        }

        offset *= p.size;

        Vertex_OUT v;
        v.color = p.endColor + (p.lifeTime / p.maxLifeTime) * (p.beginColor - p.endColor);

        glm::vec4 viewCenter = Pass().view * glm::vec4(p.pos, 1);

        float c = cos(p.rotationRad);
        float s = sin(p.rotationRad);

        glm::vec2 rotatedOffset;
        rotatedOffset.x = offset.x * c - offset.y * s;
        rotatedOffset.y = offset.x * s + offset.y * c;

        v.posModel = viewCenter;
        v.posModel.x += rotatedOffset.x;
        v.posModel.y += rotatedOffset.y;

        v.posH = Pass().proj * v.posModel;
        v.normal = _in.normal;
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
        if (_in.color.a < 0.f)
            return {};

        return pcolor().color * _in.color;
    }
};

#endif

