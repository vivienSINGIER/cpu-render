#ifndef HEAPS_HEAP_UTILS_HPP_DEFINED
#define HEAPS_HEAP_UTILS_HPP_DEFINED
#include <cstdint>

#define VERTEX_BUFFER_HEAP_ID 0
#define INDEX_BUFFER_HEAP_ID 1
#define VERTEX_SHADER_HEAP_ID 2
#define PIXEL_SHADER_HEAP_ID 3
#define SHADER_GROUP_HEAP_ID 4
#define COMPUTE_SHADER_HEAP_ID 5
#define TEXTURE_HEAP_ID 6
#define CONSTANTBUFFER_HEAP_ID 7
#define UAV_HEAP_ID 8

struct FreeRange
{
    uint32_t offset;
    uint32_t size;
};

enum class HeapState { FREE, BLOCKED };

struct UAVView { void* data; uint32_t count; uint32_t stride; };

#endif
