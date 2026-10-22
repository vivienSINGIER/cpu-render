#ifndef HEAPS_CONSTANT_BUFFER_HEAP_INL_DEFINED
#define HEAPS_CONSTANT_BUFFER_HEAP_INL_DEFINED

#include <cassert>

#include "ConstantBufferHeap.h"

template <typename T>
void ConstantBufferHeap::Write(Handle _h, T const& _data)
{
    assert(_h.Size() == sizeof(T));
    
    memcpy(m_data.data() + _h.Offset(), &_data, sizeof(T));
}

template <typename T>
T const& ConstantBufferHeap::Read(Handle _h)
{
    assert(_h.Size() == sizeof(T));
    return *reinterpret_cast<const T*>(m_data.data() + _h.Offset());
}

#endif
