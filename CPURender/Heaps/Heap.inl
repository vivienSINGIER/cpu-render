#ifndef HEAPS_HEAP_INL_DEFINED
#define HEAPS_HEAP_INL_DEFINED

#include "Heap.h"

template <typename Base>
template <typename Derived, typename ... Args>
Handle Heap<Base>::Create(Args&&... args)
{
    static_assert(std::is_base_of<Base, Derived>::value, "Derived class must be derived from Base");
    
    size_t index;
        
    if (m_free.empty())
    {
        index = m_items.size();
        m_items.push_back(std::make_unique<Derived>(std::forward<Args>(args)...));
    }
    else
    {
        index = m_free.back();
        m_free.pop_back();
        m_items[index] = std::make_unique<Derived>(std::forward<Args>(args)...);
    }
    
    return Handle::Make(m_heapId, sizeof(Derived), (uint32_t)index);
}

template <typename Base>
Base* Heap<Base>::Get(Handle _h)
{
    if (_h.HeapId() != m_heapId || _h.Offset() >= m_items.size())
        return nullptr;
    
    return m_items[_h.Offset()].get();
}

template <typename Base>
void Heap<Base>::Delete(Handle _h)
{
    if (_h.HeapId() != m_heapId || _h.Offset() >= m_items.size() || m_items[_h.Offset()] == nullptr)
        return;
    
    m_items[_h.Offset()].reset();
    m_free.push_back(_h.Offset());
}

template <typename Base>
Heap<Base>::Heap(uint16_t _heapId, GraphicsContext* _ctx)
{
    m_heapId = _heapId;
    m_ctx = _ctx;
}

#endif
