#ifndef CORE_GRAPHICS_CONTEXT_H_DEFINED
#define CORE_GRAPHICS_CONTEXT_H_DEFINED

#include "../Heaps/Heaps.h"

#include "../Shaders/ShaderBase/Shader.h"
#include "../Shaders/ShaderBase/ShaderGroup.h"

#include "../Datas/Texture.h"
#include "../Datas/Mesh.h"

class GraphicsContext
{
public:
    BufferHeap<Vertex> vertexBufferHeap;
    BufferHeap<Index> indexBufferHeap;
    
    Heap<VertexShader> vertexShaderHeap;
    Heap<PixelShader> pixelShaderHeap;
    Heap<ShaderGroup> shaderGroupHeap;
    Heap<ComputeShader> computeShaderHeap;
    
    Heap<Texture> textureHeap;
    Heap<Texture> frameBufferHeap;
    
    ConstantBufferHeap constantBufferHeap;
    UAVHeap uavHeap;
    
    void SetHeapsState(HeapState _state);
    
    Texture* GetTexture(Handle _h);

private:
    GraphicsContext();
    
    friend class Device;
};

#endif
