#ifndef FACTORIES_BUFFER_FACTORY_H_DEFINED
#define FACTORIES_BUFFER_FACTORY_H_DEFINED

#include <cstdint>

#include "../Heaps/Handle.hpp"

class GraphicsContext;

// UAV buffers: Create & Delete outside a pass, Read & Write anytime
class BufferFactory
{
public:
    static BufferFactory* Instance();

    template <typename T>
    Handle CreateUAVBuffer(uint32_t _count);
    template <typename T>
    Handle CreateUAVBuffer(uint32_t _count, T const& _fillValue);

    template <typename T>
    T const& Read(Handle _h, uint32_t _index) const;
    template <typename T>
    void Write(Handle _h, uint32_t _index, T const& _value);
    template <typename T>
    void Fill(Handle _h, T const& _value);

    void Delete(Handle _h);

private:
    static inline BufferFactory* g_instance = nullptr;
    GraphicsContext* m_ctx;

    BufferFactory(GraphicsContext* _pContext);
    static void Init(GraphicsContext* _pContext);

    friend class GraphicsContext;
};

#include "BufferFactory.inl"

#endif
