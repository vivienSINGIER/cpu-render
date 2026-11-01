#include "UAVHeap.h"

Handle UAVHeap::Create(uint16_t _stride, uint32_t _count)
{
    return Allocate(_stride, _count);
}

UAVView UAVHeap::GetView(Handle _h)
{
    uint8_t* data = GetRaw(_h);
    if (data == nullptr)
        return UAVView{};

    return UAVView{ data, _h.Count(), _h.Stride() };
}

UAVHeap::UAVHeap(uint16_t _heapId, GraphicsContext* _ctx) : RawBufferHeap(_heapId, _ctx)
{
}
