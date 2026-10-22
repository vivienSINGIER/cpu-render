#ifndef HEAPS_BUFFERHEAP_INL_DEFINED
#define HEAPS_BUFFERHEAP_INL_DEFINED

#include "BufferHeap.h"
#include <cassert>
#include <cstring>

template <typename T>
Handle BufferHeap<T>::Create(T* _data, uint16_t _count)
{
    uint32_t index = UINT32_MAX;
    uint32_t size = sizeof(T) * _count;
    
    for (size_t i = 0; i < m_free.size(); i++)
    {
        if (m_free[i].size < size)
            continue;
        index = m_free[i].offset;
        m_free[i].offset += size;
        m_free[i].size -= size;
        if (m_free[i].size == 0)
            m_free.erase(m_free.begin() + i);
        break;
    }
    
    if (index >= UINT32_MAX)
    {
        index = (uint32_t)m_data.size();
        m_data.resize(index + size);
    }
    
    memcpy(m_data.data() + index, _data, size);
    
    return Handle::Make(m_heapId, _count, index);
}

template <typename T>
T const* BufferHeap<T>::GetData(Handle _h)
{
    return reinterpret_cast<T const*>(m_data.data() + _h.Offset());
}

template <typename T>
T const* BufferHeap<T>::GetData(Handle _h, uint16_t& _count)
{
    _count = _h.Size();
    return reinterpret_cast<T const*>(m_data.data() + _h.Offset());
}

template <typename T>
void BufferHeap<T>::Delete(Handle _h)
{
    if (_h.HeapId() != m_heapId || _h.Offset() >= m_data.size())
        return;
    
    FreeRange r = { _h.Offset(), _h.Size() * (uint32_t)sizeof(T) };
    
    auto it = m_free.begin();
    while (it != m_free.end())
    {
        assert((r.size != it->size || r.offset != it->offset) && "can't delete a free handle");
        if (r.offset < it->offset)
            break;
        ++it;        
    }
    it = m_free.insert(it, r);
    
    if (it + 1 != m_free.end() && it->offset + it->size == (it + 1)->offset)
    {
        it->size += (it + 1)->size;
        m_free.erase(it + 1);
    }
    if (it != m_free.begin() && (it - 1)->offset + (it - 1)->size == it->offset)
    {
        (it - 1)->size += it->size;
        m_free.erase(it);
    }
}

template <typename T>
BufferHeap<T>::BufferHeap(uint16_t _heapId, GraphicsContext* _ctx)
{
    m_heapId = _heapId;
    m_ctx = _ctx;
}

#endif

