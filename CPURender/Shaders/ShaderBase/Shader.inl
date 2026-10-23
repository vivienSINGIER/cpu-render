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
T const& Shader::CB(ShaderResources const& _r, int _slot) const
{
    assert(_r.cbs[_slot] != nullptr && sizeof(T) == m_cb[_slot].size && "Can't access constant buffer data");
    return *reinterpret_cast<T const*>(_r.cbs[_slot]);
}

#endif
