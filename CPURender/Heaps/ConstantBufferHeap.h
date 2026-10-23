#ifndef HEAPS_CONSTANT_BUFFER_HEAP_H
#define HEAPS_CONSTANT_BUFFER_HEAP_H

#include <vector>
#include <cstddef>

#include "Handle.hpp"
#include "HeapUtils.hpp"

class GraphicsContext;

class ConstantBufferHeap
{
public:
    Handle Create(uint16_t _size);
    
    template <typename T>
    void Write(Handle _h, T const& _data);
    
    template <typename T>
    T const& Read(Handle _h);
    uint8_t const* GetRaw(Handle _h) const;
    
    void Delete(Handle _h);
    
private:
    std::vector<uint8_t> m_data;
    uint16_t m_heapId;
    GraphicsContext* m_ctx;

    std::vector<FreeRange> m_free;

    ConstantBufferHeap(uint16_t _heapId, GraphicsContext* _ctx);
    
    friend class GraphicsContext;
};

#include "ConstantBufferHeap.inl"

#endif
