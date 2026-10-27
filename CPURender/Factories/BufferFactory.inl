#ifndef FACTORIES_BUFFER_FACTORY_INL_DEFINED
#define FACTORIES_BUFFER_FACTORY_INL_DEFINED

#include "BufferFactory.h"

#include <cassert>

#include "../Core/GraphicsContext.h"

template <typename T>
Handle BufferFactory::CreateUAVBuffer(uint32_t _count)
{
    static_assert(sizeof(T) <= UINT16_MAX, "element too large for a handle stride");

    Handle h = m_ctx->uavHeap.Create(sizeof(T), _count);
    assert(h.IsValid() && "UAV buffer creation failed, is a pass in progress ?");
    return h;
}

template <typename T>
Handle BufferFactory::CreateUAVBuffer(uint32_t _count, T const& _fillValue)
{
    Handle h = CreateUAVBuffer<T>(_count);
    Fill<T>(h, _fillValue);
    return h;
}

template <typename T>
T const& BufferFactory::Read(Handle _h, uint32_t _index) const
{
    return m_ctx->uavHeap.Read<T>(_h, _index);
}

template <typename T>
void BufferFactory::Write(Handle _h, uint32_t _index, T const& _value)
{
    m_ctx->uavHeap.Write<T>(_h, _value, _index);
}

template <typename T>
void BufferFactory::Fill(Handle _h, T const& _value)
{
    for (uint32_t i = 0; i < _h.Count(); i++)
        m_ctx->uavHeap.Write<T>(_h, _value, i);
}

#endif
