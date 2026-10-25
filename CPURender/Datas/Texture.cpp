#include "Morton.hpp"
#include "Texture.h"

#include <bit>
#include <cassert>
#include <glm/common.hpp>

#include "../stb_image.h"

namespace
{
    uint32 NearestPowerOf2(uint32 _value)
    {
        uint32 n = 1;
        uint32 t = 2;
        while (t < _value)
        {
            t *= 2;
            n++;
        }
        return t;
    }
    
    Color EncodeTexel(Texture::Format _format, glm::vec4 _v)
    {
        switch (_format)
        {
        case Texture::ARGB8:
            return Color::FromVec4(glm::clamp(_v, 0.0f, 1.0f));
        case Texture::R32F:
            return std::bit_cast<Color>(_v.r);
        }

        assert(false && "Unhandled texture format");
        return Color::NONE;
    }
}

Texture::Texture(uint32 _width, uint32 _height, Format _format, Encoding _enc, Color _color) : m_encoding(_enc), m_format(_format)
{
    SetSize(_width, _height);
    
    m_data.assign(m_size, _color);
}

uint32 Texture::GetSize(uint32& _width, uint32& _height) const
{
    _width = m_width;
    _height = m_height;
    return m_size;
}

uint32 Texture::Size() const
{
    return m_size;
}

uint32 Texture::Width() const
{
    return m_width;
}

uint32 Texture::Height() const
{
    return m_height;
}

void Texture::SetSize(uint32 _width, uint32 _height)
{
    uint32 s = NearestPowerOf2(std::max(_width, _height));
    m_size = s * s;
    m_width = _width;
    m_height = _height;
    m_data.resize(m_size);
}

Color Texture::SampleNP(glm::vec2 uv) const
{
    uv.x = glm::clamp(uv.x, 0.0f, 1.0f);
    uv.y = glm::clamp(uv.y, 0.0f, 1.0f);

    uint32_t x = static_cast<uint32_t>(glm::floor(uv.x * (float)m_width));
    uint32_t y = static_cast<uint32_t>(glm::floor(uv.y * (float)m_height));
    
    if (x >= m_width)  x = m_width - 1;
    if (y >= m_height) y = m_height - 1;

    return LoadC(x, y);
}

glm::vec4 Texture::Load(uint32 _x, uint32 _y) const
{
    Color c = LoadC(_x, _y);

    switch (m_format)
    {
    case ARGB8:
        return Color::ToVec4(c);
    case R32F:
        return { std::bit_cast<float>(c), 0.0f, 0.0f, 1.0f };
    }

    assert(false && "Unhandled texture format");
    return { 0.0f, 0.0f, 0.0f, 0.0f };
}

void Texture::Store(uint32 _x, uint32 _y, glm::vec4 _v)
{
    Store(_x, _y, EncodeTexel(m_format, _v));
}

void Texture::Clear(glm::vec4 _color)
{
    Clear(EncodeTexel(m_format, _color));
}

Texture Texture::ToLinear() const
{
    Texture t(m_width, m_height, m_format, LINEAR);
    
    for (uint32 y = 0; y < m_height; ++y)
    {
        for (uint32 x = 0; x < m_width; ++x)
        {
            uint32 index = GetIndex(x, y);
            t.Store(x, y, m_data[index]);
        }
    }
    return t;
}

void Texture::Load(std::string const& _filePath)
{
    static int nbrChannel;
    int width, height;
    unsigned char* data = stbi_load(_filePath.c_str(), &width, &height, &nbrChannel, 4);
    
    assert(data != nullptr && "failed to load texture");
    
    SetSize(width, height);
    
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int linearized2D = x + y * width;
            uint32_t index = GetIndex(x, y);
            
            m_data[index].a = data[3 + linearized2D * 4];
            m_data[index].r = data[0 + linearized2D * 4];
            m_data[index].g = data[1 + linearized2D * 4];
            m_data[index].b = data[2 + linearized2D * 4];
        }
    }
    
    free(data);
}

void Texture::Clear(Color _color)
{
    m_data.assign(m_size, _color);
}

Texture::Format Texture::GetFormat() const
{
    return m_format;
}

void Texture::Store(uint32 _x, uint32 _y, Color _color)
{
    uint32 i = GetIndex(_x, _y);
    
    m_data[i] = _color;
}

Color Texture::LoadC(uint32 _x, uint32 _y) const
{
    uint32 i = GetIndex(_x, _y);
    return m_data[i];
}

Color* Texture::GetData()
{
    return m_data.data();
}

uint32 Texture::GetIndex(uint32 _x, uint32 _y) const
{
    switch (m_encoding)
    {
    case MORTON:
        return EncodeMorton2(_x, _y);
    case LINEAR:
        return _x + _y * m_width;
    }
    return 0;
}

