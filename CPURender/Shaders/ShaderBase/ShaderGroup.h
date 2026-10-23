#ifndef SHADERS_SHADER_BASE_SHADER_GROUP_H_DEFINED
#define SHADERS_SHADER_BASE_SHADER_GROUP_H_DEFINED
#include <cstdint>

#include "../../Heaps/Handle.hpp"
#include "Shader.h"

struct CBSlotDesc {uint16_t size; bool isDeclared; bool isVS; bool isPS; };
struct TexSlotDesc {bool isDeclared; bool isOptional; bool isVS; bool isPS; };

class ShaderGroup
{
public:
    void Build();
    
    CBSlotDesc CBLayout(int _slot);
    TexSlotDesc TexLayout(int _slot);
    
    ShaderStages Resolve();
    
private:
    Handle m_vs;
    Handle m_ps;
    
    std::array<CBSlotDesc, MAX_CB> m_cbLayout;
    std::array<TexSlotDesc, MAX_TEX> m_texLayout;
    
    GraphicsContext* m_ctx;
    
    ShaderGroup(GraphicsContext* _ctx, Handle _vs, Handle _ps);
    
    friend class ShaderFactory;
};

#endif
