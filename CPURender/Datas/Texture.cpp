#include "Morton.hpp"
#include "Texture.h"

#include <bit>
#include <cassert>
#include <glm/common.hpp>
#include <glm/exponential.hpp>

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
    
    vec4 EncodeTexel(Texture::Format _format, glm::vec4 _v)
    {
        switch (_format)
        {
        case Texture::RGBA32:
            return _v;
        case Texture::R32F:
            return vec4{ _v.r, 0.0f, 0.0f, 1.0f };
        }

        assert(false && "Unhandled texture format");
        return Color::NONE;
    }
}

Texture::Texture(uint32 _width, uint32 _height, Format _format, Encoding _enc, vec4 _color) : m_encoding(_enc), m_format(_format)
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

glm::vec4 Texture::SampleNP(glm::vec2 uv) const
{
    uv.x = glm::clamp(uv.x, 0.0f, 1.0f);
    uv.y = glm::clamp(uv.y, 0.0f, 1.0f);

    uint32_t x = static_cast<uint32_t>(glm::floor(uv.x * (float)m_width));
    uint32_t y = static_cast<uint32_t>(glm::floor(uv.y * (float)m_height));
    
    if (x >= m_width)  x = m_width - 1;
    if (y >= m_height) y = m_height - 1;

    return Load(x, y);
}

glm::vec4 Texture::SampleNPCube(glm::vec3 _dir) const
{
    enum Side { FRONT = 0, BACK, LEFT, RIGHT, UP, DOWN };
    Side cubemapSide;
    float absX = abs(_dir.x);
    float absY = abs(_dir.y);
    float absZ = abs(_dir.z);

    if (absX >= absY && absX >= absZ) // x major
    {
        cubemapSide = _dir.x < 0 ? LEFT : RIGHT;
    }
    else if (absY >= absX && absY >= absZ) // y major
    {
        cubemapSide = _dir.y < 0 ? DOWN : UP;
    }
    else // z major
    {
        cubemapSide = _dir.z < 0 ? BACK : FRONT;
    }

    const float s = 1.0f / 3.0f;
    int xOffset = (cubemapSide % 2);
    int yOffset = (cubemapSide / 2);
    glm::vec2 uvOffset = glm::vec2(xOffset * s, yOffset * s);
    glm::vec2 uv;
    
    float maxAbs = glm::max(absX, glm::max(absY, absZ));
    vec3 d = _dir / maxAbs;
    
    switch (cubemapSide)
    {
    case FRONT:
        uv = { (d.x + 1) / 2 , 1 - ((d.y + 1) / 2) };
        break;
    case BACK:
        uv = { (-d.x + 1) / 2 , 1 - ((d.y + 1) / 2) };
        break;
    case LEFT:
        uv = { (d.z + 1) / 2 , 1 - ((d.y + 1) / 2) };
        break;
    case RIGHT:
        uv = { (-d.z + 1) / 2 , 1 - ((d.y + 1) / 2) };
        break;
    case UP:
        uv = { (d.x + 1) / 2 , 1 - ((-d.z + 1) / 2) };
        break;
    case DOWN:
        uv = { (d.x + 1) / 2 , 1 - ((d.z + 1) / 2) };
        break;
    }

    return SampleNP(uv*s + uvOffset);
}

glm::vec4 Texture::Load(uint32 _x, uint32 _y) const
{
    vec4 c = m_data[GetIndex(_x, _y)];

    switch (m_format)
    {
    case RGBA32:
        return c;
    case R32F:
        return { c.r, 0.0f, 0.0f, 1.0f };
    }

    assert(false && "Unhandled texture format");
    return { 0.0f, 0.0f, 0.0f, 0.0f };
}

void Texture::Store(uint32 _x, uint32 _y, glm::vec4 _v)
{
    m_data[GetIndex(_x, _y)] = EncodeTexel(m_format, _v);;
}

void Texture::Clear(glm::vec4 _color)
{
    m_data.assign(m_size, EncodeTexel(m_format, _color));
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

void Texture::Load(std::string const& _filePath, bool _isSrgb)
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
            
            m_data[index].r = (float)data[0 + linearized2D * 4] / 255.0f;
            m_data[index].g = (float)data[1 + linearized2D * 4] / 255.0f;
            m_data[index].b = (float)data[2 + linearized2D * 4] / 255.0f;
            m_data[index].a = (float)data[3 + linearized2D * 4] / 255.0f;
            
            if (_isSrgb)
            {
                m_data[index].r = glm::pow(m_data[index].r, 2.2f);
                m_data[index].g = glm::pow(m_data[index].g, 2.2f);
                m_data[index].b = glm::pow(m_data[index].b, 2.2f);
                m_data[index].a = glm::pow(m_data[index].a, 2.2f);
            }
        }
    }
    
    free(data);
}

Texture::Format Texture::GetFormat() const
{
    return m_format;
}

vec4* Texture::GetData()
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

