#ifndef HEAPS_BUFFERHEAP_INL_DEFINED
#define HEAPS_BUFFERHEAP_INL_DEFINED

#include "BufferHeap.h"

template <typename T>
Handle BufferHeap<T>::Create(T const* _data, uint32_t _count)
{
    return Allocate(sizeof(T), _count, _data);
}

template <typename T>
T* BufferHeap<T>::GetData(Handle _h)
{
    return reinterpret_cast<T*>(GetRaw(_h));
}

template <typename T>
T const* BufferHeap<T>::GetData(Handle _h) const
{
    return reinterpret_cast<T const*>(GetRaw(_h));
}

template <typename T>
T* BufferHeap<T>::GetData(Handle _h, uint32_t& _count)
{
    _count = _h.Count();
    return reinterpret_cast<T*>(GetRaw(_h));
}

template <typename T>
T const* BufferHeap<T>::GetData(Handle _h, uint32_t& _count) const
{
    _count = _h.Count();
    return reinterpret_cast<T const*>(GetRaw(_h));
}

template <typename T>
BufferHeap<T>::BufferHeap(uint16_t _heapId, GraphicsContext* _ctx) : RawBufferHeap(_heapId, _ctx)
{
}

#endif
