#include "Shader.h"

#include <cassert>

#include "../../Core/GraphicsContext.h"
#include "../../Datas/ConstantBuffers.hpp"

void Shader::AddTexture(int _slot)
{
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_tex[_slot].isDeclared == false && "can't override texture register declaration");
    
    m_tex[_slot] = { true };
}

Shader::Shader(GraphicsContext* _ctx) : m_cb(), m_tex()
{
    m_ctx = _ctx;

    // reserved slots, AddConstantBuffer refuses them
    m_cb[CB_PER_PASS] = CBRegister{ sizeof(PerPassCB), true };
    m_cb[CB_PER_OBJECT] = CBRegister{ sizeof(PerObjectCB), true };
}

Color Shader::Sample(ShaderResources const& _r, int _slot, glm::vec2 _uv) const
{
    assert(_r.textures[_slot] != nullptr && "Can't access texture data");
    
    Texture const* tex = _r.textures[_slot];
    
    return tex->SampleNP(_uv);
}
