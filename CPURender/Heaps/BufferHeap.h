#ifndef HEAPS_BUFFER_HEAP_H_DEFINED
#define HEAPS_BUFFER_HEAP_H_DEFINED
#include <vector>

#include "Handle.hpp"
#include "HeapUtils.hpp"

class GraphicsContext;

template <typename T>
class BufferHeap
{
public:
    Handle Create(T* _data, uint16_t _count);
    
    T const* GetData(Handle _h);
    T const* GetData(Handle _h, uint16_t& _count);
    
    void Delete(Handle _h);
    
private:
    std::vector<uint8_t> m_data;
    uint32_t m_heapId;
    GraphicsContext* m_ctx;
    
    std::vector<FreeRange> m_free;
    
    BufferHeap(uint16_t _heapId, GraphicsContext* _ctx);
    
    friend class GraphicsContext;
};

#include "BufferHeap.inl"

#endif
