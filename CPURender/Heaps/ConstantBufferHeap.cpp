#include "ConstantBufferHeap.h"

Handle ConstantBufferHeap::Create(uint16_t _size)
{
    uint32_t index = static_cast<uint32_t>(m_data.size());
    m_data.resize(index + _size);
    
    return Handle::Make(m_heapId, _size, index);
}

void ConstantBufferHeap::Delete(Handle _h)
{
    if (_h.HeapId() != m_heapId || _h.Offset() >= m_data.size())
        return;
    
    m_data.erase(m_data.begin() + _h.Offset(), m_data.begin() + _h.Offset() + _h.Size()); 
}

ConstantBufferHeap::ConstantBufferHeap(uint16_t _heapId, GraphicsContext* _ctx)
{
    m_heapId = _heapId;
    m_ctx = _ctx;
}
