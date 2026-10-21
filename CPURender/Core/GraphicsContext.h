#ifndef CORE_GRAPHICS_CONTEXT_H_DEFINED
#define CORE_GRAPHICS_CONTEXT_H_DEFINED

#include "../Heaps/Heaps.h"

#include "../Shaders/ShaderBase/Shader.h"
#include "../Shaders/ShaderBase/ShaderGroup.h"

#include "../Datas/Texture.h"

//TODO REMOVE & Add includes
class Vertex;
using Index = uint32_t;

class GraphicsContext
{
public:
    BufferHeap<Vertex> vertexBufferHeap;
    BufferHeap<Index> indexBufferHeap;
    
    Heap<VertexShader> vertexShaderHeap;
    Heap<PixelShader> pixelShaderHeap;
    Heap<Shader> shaderGroupHeap;
    
    Heap<Texture> textureHeap;
    Heap<Texture> frameBufferHeap;
    
    ConstantBufferHeap constantBufferHeap;
    
private:
    GraphicsContext();
};

#endif
