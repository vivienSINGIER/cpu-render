#ifndef HEAPS_CONSTANT_BUFFER_HEAP_H
#define HEAPS_CONSTANT_BUFFER_HEAP_H

#include "RawBufferHeap.h"

class ConstantBufferHeap : public RawBufferHeap
{
public:
    Handle Create(uint16_t _size);

private:
    ConstantBufferHeap(uint16_t _heapId, GraphicsContext* _ctx);

    friend class GraphicsContext;
};

#endif
