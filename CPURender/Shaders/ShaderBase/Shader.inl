#ifndef SHADER_BASE_SHADER_INL_DEFINED
#define SHADER_BASE_SHADER_INL_DEFINED

#include "Shader.h"

#include <cassert>

template <typename T>
void Shader::AddConstantBuffer(int _slot)
{
    assert(_slot >= CB_CUSTOM && _slot < MAX_CB && "Constant buffer slot index out of bounds or reserved for the renderer");
    assert(m_cb[_slot].isDeclared == false && "can't override constant buffer register declaration");
    
    m_cb[_slot] = CBRegister{ sizeof(T), true};
}

template <typename T>
T const& Shader::CB(int _slot) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(m_r->cbs[_slot] != nullptr && sizeof(T) == m_cb[_slot].size && "Can't access constant buffer data");
    return *reinterpret_cast<T const*>(m_r->cbs[_slot]);
}

inline PerPassCB const& Shader::Pass() const { return CB<PerPassCB>(CB_PER_PASS); }
inline LightPassCB const& Shader::Light() const { return CB<LightPassCB>(CB_LIGHT_PASS); }
inline PerObjectCB const& Shader::Object() const { return CB<PerObjectCB>(CB_PER_OBJECT); }

#endif
