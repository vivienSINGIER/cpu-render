#ifndef CORE_FRAMEBUFFER_HPP_DEFINED
#define CORE_FRAMEBUFFER_HPP_DEFINED

#include <array>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

#include "../Datas/Color.h"
#include "../Heaps/Handle.hpp"

#define MAX_RT 8

struct FrameBuffer
{
    std::array<Handle, MAX_RT> gBuffer;
    Color clearColor;
    
    Handle depthBuffer;
    bool useDepth = true;
    
    int width;
    int height;
};

#endif
