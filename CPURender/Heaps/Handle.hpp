#ifndef HEAPS_HANDLE_HPP_DEFINED
#define HEAPS_HANDLE_HPP_DEFINED

#include <cstdint>

class Handle
{
public:
    Handle() : m_offset(0), m_count(0), m_stride(0), m_heapId(0) {}
    
    static Handle Make(uint16_t _id, uint32_t _offset, uint16_t _stride, uint32_t _count)
    {
        Handle h;
        h.m_heapId = _id;
        h.m_offset = _offset;
        h.m_stride = _stride;
        h.m_count = _count;

        return h;
    }
    
    bool IsValid() const { return m_count != 0; }

    uint16_t HeapId() const { return m_heapId; }
    uint32_t Offset() const { return m_offset; }
    uint16_t Stride() const { return m_stride; }
    uint32_t Count() const { return m_count; }
    uint32_t ByteSize() const { return (uint32_t)m_stride * m_count; }

private:
    uint32_t m_offset;
    uint32_t m_count;
    uint16_t m_stride;
    uint16_t m_heapId;
};

#endif
