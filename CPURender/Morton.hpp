#ifndef MORTON_ENCODING_HPP_DEFINED
#define MORTON_ENCODING_HPP_DEFINED

#include <glm/ext/scalar_uint_sized.hpp>

using namespace glm;


inline uint32 SplitBy1(uint32 _a)
{
    uint32 x = _a & 0x0000FFFF;
    x = (x | x << 8) & 0x00FF00FF;
    x = (x | x << 4) & 0x0F0F0F0F;
    x = (x | x << 2) & 0x33333333;
    x = (x | x << 1) & 0x55555555;
    return x;
}

inline uint32 CompactBy1(uint32 _a)
{
    uint32 x = _a & 0x55555555;
    x = (x | x >> 1) & 0x33333333;
    x = (x | x >> 2) & 0x0F0F0F0F;
    x = (x | x >> 4) & 0x00FF00FF;
    x = (x | x >> 8) & 0x0000FFFF;
    return x;
}

inline uint32 SplitBy2(uint32 _a)
{
    uint32 x = _a & 0x00003FF;
    x = (x | x << 16)   & 0xFF0000FF;
    x = (x | x << 8)    & 0x0300F00F;
    x = (x | x << 4)    & 0x030C30C3;
    x = (x | x << 2)    & 0x09249249;
    return x;
}

inline uint32 CompactBy2(uint32 _a)
{
    uint32 x = _a & 0x09249249;
    x = (x | x >> 16)   & 0x030C30C3;
    x = (x | x >> 8)    & 0x0300F00F;
    x = (x | x >> 4)    & 0xFF0000FF;
    x = (x | x >> 2)    & 0x000003FF;
    return x;
}

inline uint32 EncodeMorton2(uint32 _x, uint32 _y)
{
    return (SplitBy1(_y) << 1) + SplitBy1(_x);
}

inline void DecodeMorton2(uint32 _a, uint32& _x, uint32& _y)
{
    _x = CompactBy1(_a);
    _y = CompactBy1(_a >> 1);
}

inline uint32 EncodeMorton3(uint32 _x, uint32 _y, uint32 _z)
{
    return (SplitBy2(_z) << 2) + (SplitBy2(_y) << 1) + (SplitBy2(_x));
}

inline void DecodeMorton3(uint32 _a, uint32& _x, uint32& _y, uint32& _z)
{
    _x = CompactBy2(_a);
    _y = CompactBy2(_a >> 1);
    _z = CompactBy2(_a >> 2);
}

#endif