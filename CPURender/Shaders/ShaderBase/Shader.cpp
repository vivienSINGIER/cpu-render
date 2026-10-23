#include "Shader.h"

#include <cassert>

#include "../../Core/GraphicsContext.h"
#include "../../Datas/ConstantBuffers.hpp"
#include "../../Datas/Lights/LightData.hpp"

void Shader::AddTexture(int _slot, bool _obtional)
{
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_tex[_slot].isDeclared == false && "can't override texture register declaration");
    
    m_tex[_slot] = { true, _obtional };
}

Shader::Shader(GraphicsContext* _ctx) : m_cb(), m_tex()
{
    m_ctx = _ctx;

    // reserved slots, AddConstantBuffer refuses them
    m_cb[CB_PER_PASS] = CBRegister{ sizeof(PerPassCB), true };
    m_cb[CB_LIGHT_PASS] = CBRegister{ sizeof(LightPassCB), true };
    m_cb[CB_PER_OBJECT] = CBRegister{ sizeof(PerObjectCB), true };
}

void Shader::Bind(ShaderResources const* _r)
{
    m_r = _r;
}

bool Shader::HasTexture(int _slot) const
{
    assert(m_r && "shader executed without bound ressources");
    return m_r->textures[_slot] != nullptr;
}

Color Shader::Sample(int _slot, glm::vec2 _uv) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(m_r->textures[_slot] != nullptr && "Can't access texture data");
    
    Texture const* tex = m_r->textures[_slot];
    
    return tex->SampleNP(_uv);
}
