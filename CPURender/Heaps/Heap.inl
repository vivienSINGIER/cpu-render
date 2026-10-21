#ifndef HEAPS_HEAP_INL_DEFINED
#define HEAPS_HEAP_INL_DEFINED
#include "Heap.h"

template <typename Base>
template <typename Derived, typename ... Args>
Handle Heap<Base>::Create(Args&&... args)
{
    static_assert(std::is_base_of<Base, Derived>::value, "Derived class must be derived from Base");
    m_items.push_back(std::make_unique<Derived>(std::forward<Args>(args)...));
    return Handle::Make(m_heapId, sizeof(Derived), m_items.size() - 1);
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
    if (_h.HeapId() != m_heapId || _h.Offset() >= m_items.size())
        return;
    
    m_items.erase(m_items.begin() + _h.Offset());
}

template <typename Base>
Heap<Base>::Heap(uint16_t _heapId, GraphicsContext* _ctx)
{
    m_heapId = _heapId;
    m_ctx = _ctx;
}

#endif