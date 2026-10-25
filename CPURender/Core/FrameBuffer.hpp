#ifndef CORE_FRAMEBUFFER_HPP_DEFINED
#define CORE_FRAMEBUFFER_HPP_DEFINED

#include <array>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

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
    Color clearColor;
    
    Handle depthBuffer;
    bool useDepth = true;
    
    int width;
    int height;
};

struct PipelineState
{
    bool depthTest;
    bool depthWrite;
    
    CullMode cullMode;
};

struct RenderPass
{
    FrameBuffer* fb;
    PerPassCB view;
    PipelineState state;
    std::vector<Command> cmds;
};

#endif
