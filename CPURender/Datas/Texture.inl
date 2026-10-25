#ifndef TEXTURE_INL_DEFINED
#define TEXTURE_INL_DEFINED

#include "Texture.h"
#include "Morton.hpp"

template <typename T>
T Texture::GetPixelData(uint32 _x, uint32 _y)
{
    static_assert(sizeof(T) == sizeof(Color), "T must be 32 bit type");
    
    uint32 i = GetIndex(_x, _y);
    return std::bit_cast<T>(m_data.at(i));
}

template <typename T>
void Texture::SetPixelData(uint32 _x, uint32 _y, T _data)
{
    static_assert(sizeof(T) == sizeof(Color), "T must be 32 bit type");
    uint32 i = GetIndex(_x, _y);
    m_data.at(i) = std::bit_cast<Color>(_data);
}

#endif
