#include "ConstantBufferHeap.h"

Handle ConstantBufferHeap::Create(uint16_t _size)
{
    return Allocate(_size, 1);
}

ConstantBufferHeap::ConstantBufferHeap(uint16_t _heapId, GraphicsContext* _ctx) : RawBufferHeap(_heapId, _ctx)
{
}
