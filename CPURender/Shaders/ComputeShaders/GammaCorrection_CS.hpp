#ifndef SHADERS_COMPUTE_SHADERS_GAMMA_CORRECTION_CS_HPP_DEFINED
#define SHADERS_COMPUTE_SHADERS_GAMMA_CORRECTION_CS_HPP_DEFINED

#include "../../main.h"
#include "../ShaderBase/Shader.h"

class GammaCorr_CS : public ComputeShader
{
    SHADER_UAV_TEX(target, 0)

public:
    static constexpr uint32_t GROUP_SIZE = 32;
    static constexpr float GAMMA = 2.2f;

    GammaCorr_CS(GraphicsContext* _ctx) : ComputeShader(_ctx, { GROUP_SIZE, GROUP_SIZE, 1 }) {}

    void Execute(glm::uvec3 _id) const override
    {
        // the last row of groups overhangs the screen when HEIGHT isn't a multiple of GROUP_SIZE
        if (_id.x >= WIDTH || _id.y >= HEIGHT)
            return;
        
        glm::vec4 color = Load_target(_id.x, _id.y);
        
        glm::vec3 cColor = glm::pow(glm::vec3(color), glm::vec3(1.0f / GAMMA));
        
        Store_target(_id.x, _id.y, glm::vec4(cColor, color.a));
    }
};

#endif
