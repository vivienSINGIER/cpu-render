#ifndef SHADER_BASE_SHADER_INL_DEFINED
#define SHADER_BASE_SHADER_INL_DEFINED

#include "Shader.h"

#include <cassert>

template <typename T>
void Shader::AddConstantBuffer(uint32_t _slot)
{
    assert(_slot >= CB_CUSTOM && _slot < MAX_CB && "Constant buffer slot index out of bounds or reserved for the renderer");
    assert(m_cb[_slot].isDeclared == false && "can't override constant buffer register declaration");
    
    m_cb[_slot] = CBRegister{ sizeof(T), true};
}

template <typename T>
void Shader::AddUAV(uint32_t _slot)
{
    assert(_slot < MAX_UAV && "UAV buffer slot index out of bounds");
    assert(m_uavs[_slot].isDeclared == false && "can't override uav register declaration");
    
    m_uavs[_slot] = UAVRegister{ UAVType::BUFFER, sizeof(T), true};
}

template <typename T>
T const& Shader::CB(uint32_t _slot) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot < MAX_CB && "slot index out of bounds");
    assert(m_r->cbs[_slot] != nullptr && sizeof(T) == m_cb[_slot].size && "Can't access constant buffer data");
    return *reinterpret_cast<T const*>(m_r->cbs[_slot]);
}

template <typename T>
T const& Shader::Load(uint32_t _slot, int _index) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot < MAX_UAV && "slot index out of bounds");
    assert(m_r->uavs[_slot].stride == sizeof(T) && "Buffer data type doesn't match T");
    assert(m_r->uavs[_slot].data != nullptr && "Can't access constant buffer data");
    assert(_index >= 0 && _index < m_r->uavs[_slot].count && "index out of bounds");
    return *reinterpret_cast<T const*>(static_cast<uint8_t*>(m_r->uavs[_slot].data) + sizeof(T) * _index);
}

template <typename T>
void Shader::Store(uint32_t _slot, int _index, T const& _value) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot < MAX_UAV && "slot index out of bounds");
    assert(m_r->uavs[_slot].stride == sizeof(T) && "Buffer data type doesn't match T");
    assert(m_r->uavs[_slot].data != nullptr && "Can't access constant buffer data");
    assert(_index >= 0 && _index < m_r->uavs[_slot].count && "index out of bounds");
    return *reinterpret_cast<T*>(static_cast<uint8_t*>(m_r->uavs[_slot].data) + sizeof(T) * _index) = _value;
}

#endif
