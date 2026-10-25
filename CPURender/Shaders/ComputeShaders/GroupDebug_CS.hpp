#ifndef SHADERS_COMPUTE_SHADERS_GROUP_DEBUG_CS_HPP_DEFINED
#define SHADERS_COMPUTE_SHADERS_GROUP_DEBUG_CS_HPP_DEFINED

#include "../../main.h"
#include "../ShaderBase/Shader.h"

class GroupDebug_CS : public ComputeShader
{
    SHADER_UAV_TEX(target, 0)

public:
    static constexpr uint32_t GROUP_SIZE = 32;

    GroupDebug_CS(GraphicsContext* _ctx) : ComputeShader(_ctx, { GROUP_SIZE, GROUP_SIZE, 1 }) {}

    void Execute(glm::uvec3 _id) const override
    {
        // the last row of groups overhangs the screen when HEIGHT isn't a multiple of GROUP_SIZE
        if (_id.x >= WIDTH || _id.y >= HEIGHT)
            return;

        glm::uvec3 gs = GroupSize();
        glm::uvec3 group = _id / gs;
        glm::uvec3 local = _id % gs;

        uint32_t r = local.x * 255 / (gs.x - 1);
        uint32_t g = local.y * 255 / (gs.y - 1);
        uint32_t b = ((group.x + group.y) & 1) ? 255 : 0;
        
        Color scene = Load_target(_id.x, _id.y);
        Store_target(_id.x, _id.y, Color(255,
            (glm::uint8)((scene.r + r) / 2),
            (glm::uint8)((scene.g + g) / 2),
            (glm::uint8)((scene.b + b) / 2)));
    }
};

#endif
