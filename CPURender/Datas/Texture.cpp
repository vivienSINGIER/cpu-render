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
    return SampleNPCubeLod(_dir, 0.0f);
}

glm::vec4 Texture::SampleNPCubeLod(glm::vec3 _dir, float _lod) const
{
    uint32 face;
    vec2 uv = GetCubeSampleUV(_dir, face);
    uint32 F = CubeFaceSize();
    
    auto texelAt = [&](uint32 _l) -> uvec2
    {
        uint32 F_L = F >> _l;
        uint32 x = clamp((uint32)floor(uv.x * (float)F_L), (uint32)0, F_L - 1);
        uint32 y = clamp((uint32)floor(uv.y * (float)F_L), (uint32)0, F_L - 1);
        return CubeTexel(_l, face, x, y);
    };

    uint32 maxLod = MaxCubeLod();
    _lod = clamp(_lod, 0.0f, (float)maxLod);

    uint32 l0 = (uint32)floor(_lod);
    uint32 l1 = min(l0 + 1, maxLod);

    float blend = _lod - (float)l0;
    
    vec4 c1 = Load(texelAt(l0));
    if (blend == 0.0f)
        return c1;
    
    vec4 c2 = Load(texelAt(l1));
    
    return glm::mix(c1, c2, blend);
} 
glm::vec4 Texture::Load(uvec2 _c) const
{
    return Load(_c.x, _c.y);
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

void Texture::Store(uvec2 _c, glm::vec4 _v)
{
    Store(_c.x, _c.y, _v);
}

void Texture::Store(uint32 _x, uint32 _y, glm::vec4 _v)
{
    m_data[GetIndex(_x, _y)] = EncodeTexel(m_format, _v);;
}

void Texture::StoreCube(uint32 _lod, uint32 _face, uint32 _x, uint32 _y, vec4 _c)
{
    Store(CubeTexel(_lod, _face, _x, _y), _c);
}

void Texture::StoreCube(uint32 _lod, uint32 _face, uvec2 _uv, vec4 _c)
{
    Store(CubeTexel(_lod, _face, _uv.x, _uv.y), _c);
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
    float faceSize = (float)_src->CubeFaceSize() / (float)64;
    float lodLvl = floor(log2(faceSize));
    lodLvl = clamp(lodLvl, 0.0f, (float)_src->MaxCubeLod());
    
#pragma omp parallel for
    for (int32_t y = 0; y < _dst->Height(); ++y)
    {
        for (int32_t x = 0; x < _dst->Width(); ++x)
        {
            vec3 N;
            if (!_src->TexelToCubeDir(x, y, N))
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
                    
                    irradience += _src->SampleNPCubeLod(sampleDir, lodLvl) * cos(theta) * sin(theta);
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
    
#pragma omp parallel for collapse(2)
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

void Texture::GenerateCubeMips()
{
    uint32 faceSize = CubeFaceSize();
    
    for (int i = 1; i <= (int)MaxCubeLod(); i++)
    {
        for (int f = 0; f < 6; f++)
        {
            int32 F_L = (int32)faceSize >> i;

#pragma omp parallel for
            for (int32 x = 0; x < F_L; x++)
            {
                for (int32 y = 0; y < F_L; y++)
                {
                    vec4 sum = Load(CubeTexel(i - 1, f, 2*x, 2*y));
                    sum += Load(CubeTexel(i - 1, f, 2*x+1, 2*y));
                    sum += Load(CubeTexel(i - 1, f, 2*x+1, 2*y+1));
                    sum += Load(CubeTexel(i - 1, f, 2*x, 2*y+1));

                    sum = sum / vec4(4.0f);

                    Store(CubeTexel(i, f, x, y), sum);
                }
            }
        }
    }
}

vec3 Texture::CubeFaceUVToDir(uint32 _face, vec2 _uv)
{
    uint32 faceSize = CubeFaceSize();
    
    uint32 col = _face % 2;
    uint32 row = _face / 2;
    
    
}

bool Texture::TexelToCubeDir(uint32_t _x, uint32_t _y, vec3& _outDir)
{
    uint32 faceSize = CubeFaceSize();
    
    uint32_t col = _x / faceSize;
    uint32_t row = _y / faceSize;
    
    if (col > 1 || row > 2)
        return false;
    
    uint32_t face = col + row * 2;
    
    float a = ((float)(_x % faceSize) + 0.5f) / (float)faceSize;
    float b = ((float)(_y % faceSize) + 0.5f) / (float)faceSize;
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

uint32 Texture::CubeFaceSize() const
{
    return m_width / 3;
}

uint32 Texture::LodCount() const
{
    return (uint32)glm::log2((float)CubeFaceSize()) + 1;
}

uint32 Texture::MaxCubeLod() const
{
    return glm::min((uint32)m_mipMapLast, LodCount() - 1);
}

uvec2 Texture::CubeTexel(uint32 _lod, uint32 _face, uint32 _x, uint32 _y) const
{
    assert(_lod <= MaxCubeLod() && "Texture is too small for this level of detail");
    assert(_face < 6 && "Invalid cube face");

    uint32 F = CubeFaceSize();
    uint32 F_L = F >> _lod;
    assert(_x < F_L && _y < F_L && "Texel out of face bounds");
    
    uvec2 origin = { 0, 0 };
    if (_lod > 0)
        origin = { 2*F, 3*F - 6*F_L };
    
    return origin + uvec2(_face % 2, _face / 2) * uvec2(F_L) + uvec2(_x, _y);
}

vec2 Texture::GetCubeSampleUV(vec3 _dir, uint32& _faceIndex) const
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
    
    _faceIndex = (uint32)cubemapSide;

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
    
    return uv; // face-local [0,1], face offset is applied in CubeTexel
}

