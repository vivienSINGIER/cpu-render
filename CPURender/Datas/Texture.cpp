#include "Morton.hpp"
#include "Texture.h"

#include <bit>
#include <cassert>
#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>

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
    uint32_t index = GetIndex(_x, _y);
    vec4 c = m_data[index];

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

void Texture::BakeIrradiance(Texture* _src, Texture* _dst, uint32_t _faceSize)
{
#pragma omp parallel for
    for (int32_t y = 0; y < _dst->Height(); ++y)
    {
        for (int32_t x = 0; x < _dst->Width(); ++x)
        {
            vec3 N;
            if (!TexelToCubeDir(x, y, _faceSize, N))
                continue;
            
            vec3 up = glm::abs(N.y) < 0.999f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            vec3 right = normalize(cross(up, N));
            up = cross(N, right);
            
            vec3 irradience(0.0f);
            
            float sampleDelta = 0.05f;
            float nrSamples = 0.0f;
            
            for (float phi = 0.0f; phi < 2.0f * pi<float>(); phi += sampleDelta)
            {
                for (float theta = 0.0f; theta < 0.5f * pi<float>(); theta += sampleDelta)
                {
                    vec3 ts = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
                    vec3 sampleDir = ts.x * right + ts.y * up + ts.z * N;
                    
                    irradience += _src->SampleNPCube(sampleDir) * cos(theta) * sin(theta);
                    nrSamples += 1.0f;
                }
            }
            irradience = pi<float>() * irradience * (1.0f / nrSamples);
            _dst->Store(x, y, vec4(irradience, 1.0f));
        }
    }
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

bool Texture::TexelToCubeDir(uint32_t _x, uint32_t _y, uint32_t _faceSize, vec3& _outDir)
{
    uint32_t col = _x / _faceSize;
    uint32_t row = _y / _faceSize;
    
    if (col > 1 || row > 2)
        return false;
    
    uint32_t face = col + row * 2;
    
    float a = ((float)(_x % _faceSize) + 0.5f) / (float)_faceSize;
    float b = ((float)(_y % _faceSize) + 0.5f) / (float)_faceSize;
    float u = 2.0f * a - 1.0f;
    float v = 1.0f - b * 2.0f; // inverted uv.y
    
    switch (face)
    {
        case 0: _outDir = { u, v, 1.0f }; break;    // FRONT
        case 1: _outDir = { -u, v, -1.0f }; break;  // BACK
        case 2: _outDir = { -1.0f, v, u }; break;   // LEFT
        case 3: _outDir = { 1.0f, v, -u  }; break;  // RIGHT
        case 4: _outDir = { u, 1.0f, -v }; break;   // UP
        case 5: _outDir = { u, -1.0f, v }; break;   // DOWN
    }
    _outDir = glm::normalize(_outDir);
    return true;
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

