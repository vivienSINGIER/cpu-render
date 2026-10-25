#ifndef HEAPS_RAW_BUFFER_HEAP_H_DEFINED
#define HEAPS_RAW_BUFFER_HEAP_H_DEFINED

#include <vector>

#include "Handle.hpp"
#include "HeapUtils.hpp"

class GraphicsContext;

class RawBufferHeap
{
public:
    uint8_t* GetRaw(Handle _h);
    uint8_t const* GetRaw(Handle _h) const;

    template <typename T>
    void Write(Handle _h, T const& _data, uint32_t _index = 0);

    template <typename T>
    T const& Read(Handle _h, uint32_t _index = 0) const;

    void Delete(Handle _h);

    HeapState State() const { return m_state; }

protected:
    RawBufferHeap(uint16_t _heapId, GraphicsContext* _ctx);
    
    Handle Allocate(uint16_t _stride, uint32_t _count, void const* _init = nullptr);
    bool Owns(Handle _h) const;

    std::vector<uint8_t> m_data;
    std::vector<FreeRange> m_free;
    uint16_t m_heapId;
    GraphicsContext* m_ctx;
    HeapState m_state = HeapState::FREE;

private:
    void SetState(HeapState _state) { m_state = _state; }

    friend class GraphicsContext;
};

#include "RawBufferHeap.inl"

#endif
