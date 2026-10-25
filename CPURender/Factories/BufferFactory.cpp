#include "BufferFactory.h"

#include <cassert>

#include "../Core/GraphicsContext.h"

BufferFactory* BufferFactory::Instance()
{
    assert(g_instance != nullptr && "Buffer factory must be initialized by a GraphicsContext");
    return g_instance;
}

void BufferFactory::Delete(Handle _h)
{
    m_ctx->uavHeap.Delete(_h);
}

BufferFactory::BufferFactory(GraphicsContext* _pContext) : m_ctx(_pContext)
{
}

void BufferFactory::Init(GraphicsContext* _pContext)
{
    assert(g_instance == nullptr && "BufferFactory already initialized");
    g_instance = new BufferFactory(_pContext);
}
