#ifndef BUFFERH

#include "BufferHeap.h"

template <typename T>
Handle BufferHeap<T>::Create(T* _data, uint16_t _count)
{
    uint32_t index = m_data.size();
    uint32_t size = sizeof(T) * _count;
    
    m_data.resize(index + size);
    memcpy(m_data.data(), _data, size);
    
    return Handle::Make(m_heapId, size, index);
}

template <typename T>
T const* BufferHeap<T>::GetData(Handle _h, uint16_t& _count)
{
    _count = _h.Size() / sizeof(T);
    return reinterpret_cast<T const*>(m_data.data() + _h.Offset());
}

template <typename T>
void BufferHeap<T>::Delete(Handle _h)
{
    if (_h.HeapId() != m_heapId || _h.Offset() >= m_data.size())
        return;
    
    m_data.erase(m_data.begin() + _h.Offset(), m_data.begin() + _h.Offset() + _h.Size()); 
}

template <typename T>
BufferHeap<T>::BufferHeap(uint16_t _heapId, GraphicsContext* _ctx)
{
    m_heapId = _heapId;
    m_ctx = _ctx;
}

