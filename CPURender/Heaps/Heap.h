#ifndef HEAPS_HEAP_H_DEFINED
#define HEAPS_HEAP_H_DEFINED

#include <memory>
#include <vector>

#include "Handle.hpp"
#include "HeapUtils.hpp"

class GraphicsContext;

template <typename Base>
class Heap
{
public:
    template <typename Derived, typename... Args>
    Handle Create(Args&&... args);

    template <typename Derived>
    Handle Insert(std::unique_ptr<Derived> _item);

    Base* Get(Handle _h);
    
    void Delete(Handle _h);

    HeapState State() const { return m_state; }

private:
    std::vector<std::unique_ptr<Base>> m_items;
    std::vector<size_t> m_free;
    uint16_t m_heapId;
    GraphicsContext* m_ctx;
    HeapState m_state = HeapState::FREE;

    Heap(uint16_t _heapId, GraphicsContext* _ctx);
    void SetState(HeapState _state) { m_state = _state; }
    
    friend class GraphicsContext;
};

#include "Heap.inl"

#endif

