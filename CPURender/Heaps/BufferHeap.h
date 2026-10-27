#ifndef HEAPS_BUFFER_HEAP_H_DEFINED
#define HEAPS_BUFFER_HEAP_H_DEFINED

#include "RawBufferHeap.h"

template <typename T>
class BufferHeap : public RawBufferHeap
{
    static_assert(sizeof(T) <= UINT16_MAX, "element too large for a handle stride");

public:
    Handle Create(T const* _data, uint32_t _count);

    T const* GetData(Handle _h) const;
    T const* GetData(Handle _h, uint32_t& _count) const;

private:
    BufferHeap(uint16_t _heapId, GraphicsContext* _ctx);

    friend class GraphicsContext;
};

#include "BufferHeap.inl"

#endif
