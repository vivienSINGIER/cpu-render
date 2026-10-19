#include "Morton.hpp"
#include "Texture.h"

#include "stb_image.h"

namespace
{
    uint32 NearestPowerOf2(uint32 _value)
    {
        uint32 result = 1;
        uint32 t = 2;
        while (t < _value)
        {
            t *= 2;
            result++;
        }
        return result;
    }
}

Texture::Texture(uint32 _size, Color _color)
{
    SetSize(_size);
    
    m_data.assign(m_size, _color);
}

uint32 Texture::GetSize() const
{
    return m_size;
}

void Texture::SetSize(uint32 _size)
{
    m_size = NearestPowerOf2(_size);
    float offset = static_cast<float>(_size) / static_cast<float>(m_size);
    m_offsets = { offset, offset };
    m_data.resize(m_size);
}

void Texture::Load(std::string const& _filePath)
{
    static int nbrChannel;
    int width, height;
    unsigned char* data = stbi_load(_filePath.c_str(), &width, &height, &nbrChannel, 4);
    
    if (width == height)
        SetSize(width * width);
    else
    {
        int max = std::max(width, height);
        m_size = NearestPowerOf2(max * max);
        float offsetX = static_cast<float>(width) / static_cast<float>(max);
        float offsetY = static_cast<float>(height) / static_cast<float>(max);
        m_offsets = { offsetX, offsetY };
        m_data.resize(m_size);
    }
    
    for (int i = 0; i < width; ++i)
    {
        for (int j = 0; j < height; ++j)
        {
            int linearized2D = i + j * width;
            uint32_t index = EncodeMorton2(width, height);
            
            m_data.at(index).a = static_cast<uint8>(data[3 + linearized2D * nbrChannel] << 24);
            m_data.at(index).r = static_cast<uint8>(data[0 + linearized2D * nbrChannel] << 16);
            m_data.at(index).g = static_cast<uint8>(data[1 + linearized2D * nbrChannel] << 8);
            m_data.at(index).b = static_cast<uint8>(data[2 + linearized2D * nbrChannel]);
        }
    }
}

void Texture::Clear(Color _color)
{
    m_data.assign(m_size, _color);
}

void Texture::SetPixelColor(uint32 _x, uint32 _y, Color _color)
{
    uint32 i = EncodeMorton2(_x, _y);
    
    m_data.at(i) = _color;
}

Color Texture::GetPixelColor(uint32 _x, uint32 _y)
{
    uint32 i = EncodeMorton2(_x, _y);
    return m_data.at(i);
}

Color* Texture::GetData()
{
    return m_data.data();
}

