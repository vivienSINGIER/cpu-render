#ifndef HEAPS_HEAP_H_DEFINED
#define HEAPS_HEAP_H_DEFINED
#include <memory>
#include <vector>

#include "Handle.hpp"

class GraphicsContext;

template <typename Base>
class Heap
{
public:
    template <typename Derived, typename... Args>
    Handle Create(Args&&... args);
    
    Base* Get(Handle _h);
    
    void Delete(Handle _h);
    
protected:
    Heap(uint16_t _heapId, GraphicsContext* _ctx);
    
private:
    std::vector<std::unique_ptr<Base>> m_items;
    uint16_t m_heapId;
    GraphicsContext* m_ctx;
};

#include "Heap.inl"

#endif

