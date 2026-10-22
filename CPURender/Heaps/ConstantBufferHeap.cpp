#include "ConstantBufferHeap.h"

#include <cassert>
#include <cstring>

Handle ConstantBufferHeap::Create(uint16_t _size)
{
    assert(_size > 0 && "can't create an empty constant buffer");

    uint32_t index = UINT32_MAX;

    for (size_t i = 0; i < m_free.size(); i++)
    {
        if (m_free[i].size < _size)
            continue;
        index = m_free[i].offset;
        m_free[i].offset += _size;
        m_free[i].size -= _size;
        if (m_free[i].size == 0)
            m_free.erase(m_free.begin() + i);
        break;
    }

    if (index == UINT32_MAX)
    {
        index = static_cast<uint32_t>(m_data.size());
        m_data.resize(index + _size);
    }
    else
    {
        // reused range still holds the previous buffer's bytes
        memset(m_data.data() + index, 0, _size);
    }

    return Handle::Make(m_heapId, _size, index);
}

uint8_t const* ConstantBufferHeap::GetRaw(Handle _h) const
{
    if (_h.HeapId() != m_heapId || _h.Offset() >= m_data.size())
        return nullptr;
    
    return m_data.data() + _h.Offset();
}

void ConstantBufferHeap::Delete(Handle _h)
{
    if (_h.HeapId() != m_heapId || _h.Offset() >= m_data.size())
        return;
    
    FreeRange r = { _h.Offset(), _h.Size() };

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

ConstantBufferHeap::ConstantBufferHeap(uint16_t _heapId, GraphicsContext* _ctx)
{
    m_heapId = _heapId;
    m_ctx = _ctx;
}
