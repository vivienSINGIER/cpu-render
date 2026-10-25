#ifndef SHADERS_SHADER_BASE_SHADER_GROUP_H_DEFINED
#define SHADERS_SHADER_BASE_SHADER_GROUP_H_DEFINED
#include <cstdint>

#include "../../Heaps/Handle.hpp"
#include "Shader.h"

class ShaderGroup
{
public:
    ResourceLayout Build();
    ShaderStages Resolve();
    
private:
    Handle m_vs;
    Handle m_ps;
    
    GraphicsContext* m_ctx;
    
    ShaderGroup(GraphicsContext* _ctx, Handle _vs, Handle _ps);
    
    friend class ShaderFactory;
};

#endif
