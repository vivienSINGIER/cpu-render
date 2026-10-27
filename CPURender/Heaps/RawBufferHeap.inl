#ifndef HEAPS_RAW_BUFFER_HEAP_INL_DEFINED
#define HEAPS_RAW_BUFFER_HEAP_INL_DEFINED

#include <cassert>
#include <cstring>

#include "RawBufferHeap.h"

template <typename T>
void RawBufferHeap::Write(Handle _h, T const& _data, uint32_t _index)
{
    assert(Owns(_h) && "handle doesn't belong to this heap");
    assert(_h.Stride() == sizeof(T) && "buffer stride doesn't match T");
    assert(_index < _h.Count() && "index out of bounds");

    memcpy(m_data.data() + _h.Offset() + sizeof(T) * _index, &_data, sizeof(T));
}

template <typename T>
T const& RawBufferHeap::Read(Handle _h, uint32_t _index) const
{
    assert(Owns(_h) && "handle doesn't belong to this heap");
    assert(_h.Stride() == sizeof(T) && "buffer stride doesn't match T");
    assert(_index < _h.Count() && "index out of bounds");

    return *reinterpret_cast<T const*>(m_data.data() + _h.Offset() + sizeof(T) * _index);
}

#endif
