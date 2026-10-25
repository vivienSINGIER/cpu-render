#include "GraphicsContext.h"

#include "../Factories/BufferFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"

GraphicsContext::GraphicsContext() :
    vertexBufferHeap(VERTEX_BUFFER_HEAP_ID, this), 
    indexBufferHeap(INDEX_BUFFER_HEAP_ID, this),
    vertexShaderHeap(VERTEX_SHADER_HEAP_ID, this), 
    pixelShaderHeap(PIXEL_SHADER_HEAP_ID, this),
    shaderGroupHeap(SHADER_GROUP_HEAP_ID, this), 
    computeShaderHeap(COMPUTE_SHADER_HEAP_ID, this),
    textureHeap(TEXTURE_HEAP_ID, this),
    constantBufferHeap(CONSTANTBUFFER_HEAP_ID, this),
    uavHeap(UAV_HEAP_ID, this)
{
    GeometryFactory::Init(this);
    ShaderFactory::Init(this);
    TextureFactory::Init(this);
    BufferFactory::Init(this);
}

void GraphicsContext::SetHeapsState(HeapState _state)
{
    vertexBufferHeap.SetState(_state);
    indexBufferHeap.SetState(_state);
    vertexShaderHeap.SetState(_state);
    pixelShaderHeap.SetState(_state);
    shaderGroupHeap.SetState(_state);
    textureHeap.SetState(_state);
    constantBufferHeap.SetState(_state);
    uavHeap.SetState(_state);
    computeShaderHeap.SetState(_state);
}

Texture* GraphicsContext::GetTexture(Handle _h)
{
    assert((_h.HeapId() == _h.HeapId() == TEXTURE_HEAP_ID) && "invalid heap id");
    
    return textureHeap.Get(_h);
}
