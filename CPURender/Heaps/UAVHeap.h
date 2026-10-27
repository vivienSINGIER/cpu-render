#ifndef HEAPS_UAV_HEAP_H_DEFINED
#define HEAPS_UAV_HEAP_H_DEFINED

#include "RawBufferHeap.h"

// buffer UAVs only, texture UAVs live in the texture heap
class UAVHeap : public RawBufferHeap
{
public:
    Handle Create(uint16_t _stride, uint32_t _count);

    UAVView GetView(Handle _h);

private:
    UAVHeap(uint16_t _heapId, GraphicsContext* _ctx);

    friend class GraphicsContext;
};

#endif
