#ifndef CORE_FRAMEBUFFER_HPP_DEFINED
#define CORE_FRAMEBUFFER_HPP_DEFINED

#include <array>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

#include "Blending.hpp"
#include "../Datas/Color.h"
#include "../Datas/ConstantBuffers.hpp"
#include "../Datas/DrawCommand.h"
#include "../Heaps/Handle.hpp"

#define MAX_RT 8

enum CullMode
{
    FRONT, BACK, NONE
};

struct FrameBuffer
{
    std::array<Handle, MAX_RT> gBuffer;
    glm::vec4 clearColor;
    
    Handle depthBuffer;
    bool useDepth = true;
    
    uint32_t width = 0;
    uint32_t height = 0;
};

struct PipelineState
{
    bool depthTest = true;
    bool depthWrite = true;
    
    CullMode cullMode = CullMode::BACK;
    Blending::BlendMode blendMode = Blending::BlendMode::OPAQUE;
};

struct ClearFlags
{
    std::array<bool, MAX_RT> clearGBuffer = { true };
    bool clearDepth = true;
};

struct PassDesc
{
    FrameBuffer fb;
    PerPassCB passData;
    PipelineState state;
    ClearFlags clearFlags;
    bool isCompute = false;
};

struct RenderPass
{
    PassDesc desc;
    std::vector<Command> cmds;
};

#endif
