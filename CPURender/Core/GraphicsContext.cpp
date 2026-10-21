#include "GraphicsContext.h"

GraphicsContext::GraphicsContext() :
    vertexBufferHeap(0, this), indexBufferHeap(1, this),
    vertexShaderHeap(2, this), pixelShaderHeap(3, this),
    shaderGroupHeap(4, this), textureHeap(5, this), 
    frameBufferHeap(6, this), constantBufferHeap(7, this)
{
}
