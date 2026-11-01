#ifndef SHADERS_COMPUTE_SHADERS_TEX_COPY_CS_HPP_DEFINED
#define SHADERS_COMPUTE_SHADERS_TEX_COPY_CS_HPP_DEFINED

#include "../../main.h"
#include "../ShaderBase/Shader.h"

class TexCopy_CS : public ComputeShader
{
    SHADER_UAV_TEX(source, 0)
    SHADER_UAV_TEX(target, 1)

public:
    static constexpr uint32_t GROUP_SIZE = 32;

    TexCopy_CS(GraphicsContext* _ctx) : ComputeShader(_ctx, { GROUP_SIZE, GROUP_SIZE, 1 }) {}

    void Execute(glm::uvec3 _id) const override
    {
        // the last row of groups overhangs the screen when HEIGHT isn't a multiple of GROUP_SIZE
        if (_id.x >= WIDTH || _id.y >= HEIGHT)
            return;

        Store_target(_id.x, _id.y, Load_source(_id.x, _id.y));
    }
};

#endif
