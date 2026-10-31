#ifndef SHADERS_COMPUTE_SHADERS_FB_FUSION_CS_HPP_DEFINED
#define SHADERS_COMPUTE_SHADERS_FB_FUSION_CS_HPP_DEFINED

#include "../../main.h"
#include "../ShaderBase/Shader.h"

class FB_Fusion_CS : public ComputeShader
{
    SHADER_UAV_TEX(target1, 0)
    SHADER_UAV_TEX(target2, 1)
    SHADER_UAV_TEX(target1db, 2)
    SHADER_UAV_TEX(target2db, 3)

public:
    static constexpr uint32_t GROUP_SIZE = 32;

    FB_Fusion_CS(GraphicsContext* _ctx) : ComputeShader(_ctx, { GROUP_SIZE, GROUP_SIZE, 1 }) {}

    void Execute(glm::uvec3 _id) const override
    {
        // the last row of groups overhangs the screen when HEIGHT isn't a multiple of GROUP_SIZE
        if (_id.x >= WIDTH || _id.y >= HEIGHT)
            return;

        glm::vec4 color = { 1, 0, 1, 1 };

        float depth1 = Load_target1db(_id.x, _id.y).x;
        float depth2 = Load_target2db(_id.x, _id.y).x;
        
        if (depth1 > depth2) color = Load_target2(_id.x, _id.y);
        else color = Load_target1(_id.x, _id.y);

        Store_target1(_id.x, _id.y, color);
    }
};

#endif
