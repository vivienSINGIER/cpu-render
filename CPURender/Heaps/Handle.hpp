#ifndef HEAPS_HANDLE_HPP_DEFINED
#define HEAPS_HANDLE_HPP_DEFINED

#include <cstdint>
#include <memory>

class Handle
{
public:
    Handle() : m_heapId(0), m_heapOffset(0), m_size(0) {}
    
    static Handle Make(uint16_t _id, uint16_t _size, uint32_t _offset)
    {
        Handle h;
        h.m_heapId = _id;
        h.m_size = _size;
        h.m_heapOffset = _offset;
        
        return h;
    }
    
    uint64_t Raw() const
    {
        uint64_t res;
        memcpy(&res, this, sizeof(res));
        return res;
    }
    
    bool IsValid() const
    {
        return m_heapId != 0 || m_heapOffset != 0 || m_size != 0;
    }
    
    uint16_t HeapId() const { return m_heapId; }
    uint16_t Size() const { return m_size; }
    uint32_t Offset() const { return m_heapOffset; }
    
private:
    
    uint32_t m_heapOffset;
    uint16_t m_size;
    uint16_t m_heapId;
};

#endif
