#ifndef SHADERS_COMPUTE_SHADERS_TEST_CS_HPP_DEFINED
#define SHADERS_COMPUTE_SHADERS_TEST_CS_HPP_DEFINED

#include "../../main.h"
#include "../ShaderBase/Shader.h"

class Test_CS : public ComputeShader
{
    SHADER_UAV_TEX(target, 0)

public:
    static constexpr uint32_t GROUP_SIZE = 32;

    Test_CS(GraphicsContext* _ctx) : ComputeShader(_ctx, { GROUP_SIZE, GROUP_SIZE, 1 }) {}

    void Execute(glm::uvec3 _id) const override
    {
        // the last row of groups overhangs the screen when HEIGHT isn't a multiple of GROUP_SIZE
        if (_id.x >= WIDTH || _id.y >= HEIGHT)
            return;

        glm::uvec3 gs = GroupSize();
        glm::uvec3 group = _id / gs;
        glm::uvec3 local = _id % gs;

        float r = (float)local.x / (float)(gs.x - 1);
        float g = (float)local.y / (float)(gs.y - 1);
        float b = ((group.x + group.y) & 1) ? 1.0 : 0.0;

        glm::vec4 scene = Load_target(_id.x, _id.y);
        Store_target(_id.x, _id.y, { (scene.r + r) / 2.0f, (scene.g + g) / 2.0f, (scene.b + b) / 2.0f, 1.0f });
    }
};

#endif