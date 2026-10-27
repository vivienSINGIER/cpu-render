#include "RawBufferHeap.h"

#include <cassert>
#include <cstring>

uint8_t* RawBufferHeap::GetRaw(Handle _h)
{
    return Owns(_h) ? m_data.data() + _h.Offset() : nullptr;
}

uint8_t const* RawBufferHeap::GetRaw(Handle _h) const
{
    return Owns(_h) ? m_data.data() + _h.Offset() : nullptr;
}

void RawBufferHeap::Delete(Handle _h)
{
    assert(m_state == HeapState::FREE && "can't delete while the heap is blocked");
    if (m_state == HeapState::BLOCKED || Owns(_h) == false)
        return;

    FreeRange r = { _h.Offset(), _h.ByteSize() };

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

RawBufferHeap::RawBufferHeap(uint16_t _heapId, GraphicsContext* _ctx)
{
    m_heapId = _heapId;
    m_ctx = _ctx;
}

Handle RawBufferHeap::Allocate(uint16_t _stride, uint32_t _count, void const* _init)
{
    assert(_stride > 0 && _count > 0 && "can't create an empty buffer");
    assert(m_state == HeapState::FREE && "can't create while the heap is blocked");
    if (m_state == HeapState::BLOCKED)
        return Handle();

    assert((uint64_t)_stride * _count <= UINT32_MAX && "buffer too large");
    uint32_t size = (uint32_t)_stride * _count;
    uint32_t index = UINT32_MAX;
    bool reused = false;

    for (size_t i = 0; i < m_free.size(); i++)
    {
        if (m_free[i].size < size)
            continue;
        index = m_free[i].offset;
        m_free[i].offset += size;
        m_free[i].size -= size;
        if (m_free[i].size == 0)
            m_free.erase(m_free.begin() + i);
        reused = true;
        break;
    }

    if (reused == false)
    {
        assert((uint64_t)m_data.size() + size <= UINT32_MAX && "heap exceeds 4GB");
        index = static_cast<uint32_t>(m_data.size());
        m_data.resize(index + size);
    }

    if (_init != nullptr)
        memcpy(m_data.data() + index, _init, size);
    else if (reused)
        memset(m_data.data() + index, 0, size);

    return Handle::Make(m_heapId, index, _stride, _count);
}

bool RawBufferHeap::Owns(Handle _h) const
{
    return _h.IsValid() && _h.HeapId() == m_heapId
        && (uint64_t)_h.Offset() + _h.ByteSize() <= m_data.size();
}
