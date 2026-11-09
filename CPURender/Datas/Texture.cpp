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
#include "Lights/LightHelpers.hpp"
#include "Lights/PBRHelpers.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include "../stb_image_write.h"

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
    m_side = NearestPowerOf2(std::max(_width, _height));
    m_size = m_side * m_side;
    m_width = _width;
    m_height = _height;
    m_mipCount = 1;
    m_data.resize(m_size);
    
    assert(m_width % 2 == 0 && m_height % 2 == 0 && "Can't init texture with uneven dimensions");
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

glm::vec4 Texture::SampleBilinear(glm::vec2 _uv, uint32 _lod) const
{
    uvec2 dims = MipDims(_lod);
    
    vec2 p  = (_uv - floor(_uv)) * vec2(dims) - 0.5f;
    vec2 base = glm::floor(p);
    vec2 f = p - base;
    
    int32 x0 = (int32)base.x;
    int32 y0 = (int32)base.y;
    int32 x1 = x0 + 1;
    int32 y1 = y0 + 1;
    
    if (x0 < 0) 
        x0 += (int32)dims.x;
    if (y0 < 0)
        y0 += (int32)dims.y;
    if (x1 >= (int32)dims.x)
        x1 -= (int32)dims.x;
    if (y1 >= (int32)dims.y)
        y1 -= (int32)dims.y;
    
    vec4 top = glm::mix(Load(x0, y0, _lod), Load(x1, y0, _lod), f.x);
    vec4 bot = glm::mix(Load(x0, y1, _lod), Load(x1, y1, _lod), f.x);
    return glm::mix(top, bot, f.y);
}

glm::vec4 Texture::SampleLod(glm::vec2 _uv, float _lod) const
{
    if (m_mipCount == 1)                                                                                                                                                                                                     
          return SampleNP(_uv);                                                                                                                                                                                                
                                                                                                                                                                                                                               
    _lod = glm::clamp(_lod, 0.0f, (float)(m_mipCount - 1));                                                                                                                                                                  
    uint32 l0 = (uint32)_lod;                                                                                                                                                                                                
    uint32 l1 = glm::min(l0 + 1, m_mipCount - 1);                                                                                                                                                                            
    float blend = _lod - (float)l0;                                                                                                                                                                                          
                                                                                                                                                                                                                           
    vec4 c0 = SampleBilinear(_uv, l0);                                                                                                                                                                                       
    if (blend == 0.0f)                                                                                                                                                                                                       
      return c0;                                                                                                                                                                                                           
                                                                                                                                                                                                                           
    return glm::mix(c0, SampleBilinear(_uv, l1), blend);
}

glm::vec4 Texture::SampleGrad(glm::vec2 _uv, glm::vec2 _ddx, glm::vec2 _ddy) const
{
    vec2 size = vec2(m_width, m_height);
    float lx = glm::length(_ddx * size); 
    float ly = glm::length(_ddy * size);
    float pMax = glm::max(lx, ly);
    float pMin = glm::max(glm::min(lx, ly), 1e-6f);

    if (pMax == 0.0f) 
        return SampleLod(_uv, 0);

    float n   = glm::min(glm::ceil(pMax / pMin), 2.0f); // max 2x aniso for performance

    float lod = glm::log2(glm::max(pMax / n, 1e-6f));
    if (n <= 1.0f) return SampleLod(_uv, lod);

    vec2 axis = lx > ly ? _ddx : _ddy;
    vec4 sum(0.0f);
    for (int i = 0; i < (int)n; ++i)
        sum += SampleLod(_uv + axis * ((i + 0.5f) / n - 0.5f), lod);
    return sum / n;
}

glm::vec4 Texture::SampleCube(glm::vec3 _dir) const
{
    return SampleCubeLod(_dir, 0.0f);
}

glm::vec4 Texture::SampleCubeLod(glm::vec3 _dir, float _lod) const
{
    uint32 face;
    vec2 uv = GetCubeSampleUV(_dir, face);
    uint32 F = CubeFaceSize();
    
    auto texelAt = [&](uint32 _l) -> vec4
    {
        uint32 F_L = F >> _l;
        
        // Cube faces must NOT wrap: wrapping would blend texels from the opposite edge of the same face
        // and create a visible seam. Clamp to the face edge instead (GL_CLAMP_TO_EDGE behaviour).
        vec2 p  = glm::clamp(uv, 0.0f, 1.0f) * vec2((float)F_L) - 0.5f;
        vec2 base = glm::floor(p);
        vec2 f = p - base;

        int32 maxT = (int32)F_L - 1;
        int32 x0 = glm::clamp((int32)base.x,     0, maxT);
        int32 y0 = glm::clamp((int32)base.y,     0, maxT);
        int32 x1 = glm::clamp((int32)base.x + 1, 0, maxT);
        int32 y1 = glm::clamp((int32)base.y + 1, 0, maxT);
        
        vec4 top = glm::mix(Load(CubeTexel(_l, face, x0, y0)), Load(CubeTexel(_l, face, x1, y0)), f.x);
        vec4 bot = glm::mix(Load(CubeTexel(_l, face, x0, y1)), Load(CubeTexel(_l, face, x1, y1)), f.x);
        return glm::mix(top, bot, f.y);
    };

    uint32 maxLod = MaxCubeLod();
    _lod = clamp(_lod, 0.0f, (float)maxLod);

    uint32 l0 = (uint32)floor(_lod);
    uint32 l1 = min(l0 + 1, maxLod);

    float blend = _lod - (float)l0;
    
    vec4 c1 = texelAt(l0);
    if (blend == 0.0f)
        return c1;
    
    vec4 c2 = texelAt(l1);
    
    return glm::mix(c1, c2, blend);
} 
glm::vec4 Texture::Load(uvec2 _c) const
{
    return Load(_c.x, _c.y);
}

glm::vec4 Texture::Load(uvec2 _c, uint32 _lod) const
{
    return Load(_c.x, _c.y, _lod);
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

glm::vec4 Texture::Load(uint32 _x, uint32 _y, uint32 _lod) const
{
    vec4 c = m_data[GetIndex(_x, _y, _lod)];

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

void Texture::Store(uvec2 _c, uint32 _lod, glm::vec4 _v)
{
    Store(_c.x, _c.y, _lod, _v);
}

void Texture::Store(uint32 _x, uint32 _y, glm::vec4 _v)
{
    m_data[GetIndex(_x, _y)] = EncodeTexel(m_format, _v);
}

void Texture::Store(uint32 _x, uint32 _y, uint32 _lod, glm::vec4 _v)
{
    m_data[GetIndex(_x, _y, _lod)] = EncodeTexel(m_format, _v);
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
    auto const texel = EncodeTexel(m_format, _color);
    int64_t count = (int64_t)m_data.size();
#pragma omp parallel for
    for (int64_t i = 0; i < count; i++)
        m_data[i] = texel;
}

void Texture::StoreAsHDR(std::string _name, uint32_t _lod)
{
    uint32_t sideLength = MipSide(_lod);

    stbi_write_hdr(_name.c_str(), sideLength, sideLength, 4, (float*)GetMipLinear(_lod).m_data.data());
}

void Texture::StoreAsHDR(std::string _name)
{
    stbi_write_hdr(_name.c_str(), m_width, m_height, 4, (float*)ToLinear().m_data.data());
}

void Texture::StoreAtlasHDR(std::vector<Texture*> _textures)
{
    int texCount = _textures.size();
    size_t sideLength = ceil(sqrt(texCount));
    glm::i32vec2 size = glm::i32vec2(_textures[0]->m_width, _textures[0]->m_height);
    for (int i = 0; i < texCount; i++)
    {
        assert(size == glm::i32vec2(_textures[0]->m_width, _textures[0]->m_height) && "Texture size is not the same for every texture in the atlas");
    }

    Texture t(size.x * sideLength, size.y * sideLength, _textures[0]->m_format, LINEAR);

    for (int i = 0; i < texCount; i++)
    {
        int xOffset = i % sideLength * size.x;
        int yOffset = i / sideLength * size.x;
        for (int j = 0; j < size.x * size.y; j++)
        {
            int x = j % size.x;
            int y = j / size.x;
            t.SetPixelData(xOffset + x, yOffset + y, _textures[i]->GetPixelData<glm::vec4>(x, y));
        }
    }

    t.StoreAsHDR("Atlas.hdr");
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

Texture Texture::GetMipLinear(uint32_t _lod) const
{
    uint32_t side = MipSide(_lod);
    Texture t(side, side, m_format, LINEAR);
    for (uint32 y = 0; y < side; ++y)
    {
        for (uint32 x = 0; x < side; ++x)
        {
            uint32 index = GetIndex(x, y, _lod);
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
                    
                    irradience += _src->SampleCubeLod(sampleDir, lodLvl) * cos(theta) * sin(theta);
                    nrSamples += 1.0f;
                }
            }
            irradience = pi<float>() * irradience * (1.0f / nrSamples);
            _dst->Store(x, y, vec4(irradience, 1.0f));
        }
    }
}

void Texture::BakePrefilter(Texture* _src, Texture* _dst)
{
    int envSize = (int)_src->CubeFaceSize();
    int outSize = (int)_dst->CubeFaceSize();
    
    constexpr int SAMPLE_COUNT = 256;
    constexpr int MIP_COUNT = 5;
    
    uint32 lodLvl = (uint32)floor(log2(float(envSize) / float(outSize)));
    lodLvl = clamp(lodLvl, (uint32)0, _src->MaxCubeLod());

    struct Sample { vec3 L; float weight; float lod; };
    
#pragma omp parallel for collapse(2)
    for (int i = 0; i < 6; ++i)
    {
        for (int x = 0; x < outSize; ++x)
        {
            for (int y = 0; y < outSize; ++y)
            {
                uvec2 sPos = _src->CubeTexel(lodLvl, i, x, y);
                uvec2 dPos = _dst->CubeTexel(0, i, x, y);
                
                _dst->Store(dPos, _src->Load(sPos));   
            }
        }   
    }
    
    for (int m = 1; m < MIP_COUNT; ++m)
    {
        std::vector<Sample> samples;
        float roughness = (float)m / (float)(MIP_COUNT - 1);
        
        const vec3 N(0.0f, 0.0f, 1.0f);
        const float angle = 4.0f * glm::pi<float>() / (6.0f * float(envSize) * float(envSize));
        
        float totalWeight = 0.0f;
        
        for (int s = 0; s < SAMPLE_COUNT; ++s)
        {
            vec2 Xi = Lighting::Hammersley(s, SAMPLE_COUNT);
            vec3 H = Lighting::ImportanceSampleGGX(Xi, N, roughness);
            vec3 L  = normalize(vec3(2.0f * dot(N, H)) * H - N);

            float NdotL = L.z;
            if(NdotL <= 0.0)
                continue;
            
            float pdf               = Lighting::DistributionGGX(N, H, roughness) * 0.25f;
            float sampleSolidAngle  = 1.0f / (float(SAMPLE_COUNT) * pdf + 1e-6f);
            float lod               = 0.5f * glm::log2(sampleSolidAngle / angle) + 1.0f; 
            
            samples.push_back({ L, NdotL, clamp(lod, 0.0f, (float)_src->MaxCubeLod()) });
            totalWeight += NdotL;
        }
        
        const float invTotalWeight = 1.0f / totalWeight;
        
        uint32 mipSize = outSize >> m;
#pragma omp parallel for collapse(2)
        for (int i = 0; i < 6; ++i)
        {
            for (int y = 0; y < (int)mipSize; ++y)
            {
                for (int x = 0; x < (int)mipSize; ++x)
                {
                    uvec2 tx = _dst->CubeTexel(m, i, x, y);
                    
                    vec3 Nt;
                    Nt = _dst->CubeFaceUVToDir(i, vec2(x + 0.5f, y + 0.5f) / (float)mipSize);
                    Nt = normalize(Nt);
                    
                    vec3 up    = glm::abs(Nt.y) < 0.999f ? vec3(0, 1, 0) : vec3(1, 0, 0);
                    vec3 right = normalize(cross(up, Nt));
                    up         = cross(Nt, right);
                    
                    // float a = 2.0f * pi<float>() * Lighting::HashToFloat(i, m, x, y);
                    // float c = cos(a);
                    // float s = sin(a);
                    
                    vec3 color(0.0f);
                    for (auto& smp : samples)
                    {
                        // float lx = c * smp.L.x - s * smp.L.y;
                        // float ly = c * smp.L.z - s * smp.L.x;
                        
                        vec3 l = normalize(right * smp.L.x + up * smp.L.y + Nt * smp.L.z);
                        color += _src->SampleCubeLod(l, smp.lod) * smp.weight;
                    }
                    color = color * invTotalWeight;
                    
                    _dst->Store(tx, vec4(color, 1.0f));
                }
            }
        }
    }
}

void Texture::BakeBRDFLookup(Texture* _src, uint32_t _sampleCount)
{
    uint32 w = _src->Width();
    uint32 h = _src->Height();
    
#pragma omp parallel for collapse(2)
    for (int y = 0; y < (int)h; ++y)
    {
        for (int x = 0; x < (int)w; ++x)
        {
            float NdotV = ((float)x + 0.5f) / (float)w;
            float roughness = ((float)y + 0.5f) / (float)h;
            
            vec3 V = { sqrt(1.0f - NdotV*NdotV), 0.0f, NdotV };
            float A = 0.0f;
            float B = 0.0f;
            
            vec3 N = vec3(0.0f, 0.0f, 1.0f);
            
            for (uint32 i = 0; i < _sampleCount; ++i)
            {
                vec2 Xi = Lighting::Hammersley(i, _sampleCount);
                vec3 H = Lighting::ImportanceSampleGGX(Xi, N, roughness);
                vec3 L = normalize(2.0f * dot(V, H) * H - V);
                
                float NdotL = max(L.z, 0.0f);
                float NdotH = max(H.z, 0.0f);
                float VdotH = max(dot(V, H), 0.0f);
                
                if (NdotL > 0.0f)
                {
                    float G = Lighting::GeometrySmith_IBL(N, V, L, roughness);
                    float G_vis = (G * VdotH) / (NdotH * NdotV);
                    float Fc = pow(1.0f - VdotH, 5.0f);
                    
                    A += (1.0f - Fc) * G_vis;
                    B += Fc * G_vis;
                }
            }
            
            A /= (float)_sampleCount;
            B /= (float)_sampleCount;
            
            _src->Store(x, y, vec4(A, B, 0.0f, 1.0f));
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
#ifndef NDEBUG
    m_data.assign(m_data.size(), vec4(1.0f, 0.0f, 1.0f, 1.0f)); // magenta padding
#endif

    
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

void Texture::GenerateMips()
{
    assert(m_encoding == MORTON && "Mips require morton encoding");
    
    m_mipCount = MaxLod() + 1;
    m_data.resize(MipChainSize());
    
    for (uint32 l = 1; l < m_mipCount; l++)
    {
        uvec2 dst = MipDims(l);
        uvec2 src = MipDims(l - 1);
        
        int32 dstX = (int32)dst.x;
        int32 dstY = (int32)dst.y;
        
#pragma omp parallel for collapse(2)
        for (int32 y = 0; y < dstY; ++y)
        {
            for (int32 x = 0; x < dstX; ++x)
            {
                uint32 x0 = 2 * (uint32)x;
                uint32 y0 = 2 * (uint32)y;
                uint32 x1 = glm::min(x0 + 1, src.x - 1);
                uint32 y1 = glm::min(y0 + 1, src.y - 1);

                vec4 sum =  Load(x0, y0, l - 1);
                sum +=      Load(x1, y0, l - 1);
                sum +=      Load(x0, y1, l - 1);
                sum +=      Load(x1, y1, l - 1);
                
                Store(x, y, l, sum / 4.0f);
            }
        }
    }
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

vec3 Texture::CubeFaceUVToDir(uint32 _face, vec2 _uv) const
{
    vec3 dir = vec3(0.0f, 0.0f, 1.0f);
    vec2 uv = _uv * 2.0f - 1.0f;
    uv.y = -uv.y;
    
    switch (_face)
    {
    case 0: dir = { uv.x, uv.y, 1.0f }; break;    // FRONT
    case 1: dir = { -uv.x, uv.y, -1.0f }; break;  // BACK
    case 2: dir = { -1.0f, uv.y, uv.x }; break;   // LEFT
    case 3: dir = { 1.0f, uv.y, -uv.x  }; break;  // RIGHT
    case 4: dir = { uv.x, 1.0f, -uv.y }; break;   // UP
    case 5: dir = { uv.x, -1.0f, uv.y }; break;   // DOWN
    }
    
    return dir;
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

uint32 Texture::GetIndex(uint32 _x, uint32 _y, uint32 _lod) const
{
    assert(m_encoding == MORTON && "Mips require morton encoding");
    assert(_lod < m_mipCount && "Lod not generated");
    assert(_x < MipDims(_lod).x && _y < MipDims(_lod).y && "Texel out of the level, reading padding");
    return MipOffset(_lod) + EncodeMorton2(_x, _y); 
}

uint32 Texture::LodCount() const
{
    return (uint32)floor(glm::log2((float)glm::max(m_width, m_height))) + 1;
}

uint32 Texture::MaxLod() const
{
    return glm::min((uint32)m_mipMapMax, LodCount() - 1);
}

uvec2 Texture::MipDims(uint32 _lod) const
{
    return glm::max(uvec2(1), uvec2(m_width, m_height) >> _lod);
}

uint32 Texture::MipSide(uint32 _lod) const
{
    return m_side >> _lod;
}

uint32 Texture::MipOffset(uint32 _lod) const
{
    uint32 s2 = m_side * m_side;
    uint32 l2 = MipSide(_lod) * MipSide(_lod);
    return 4 * (s2 - l2) / 3;
}

uint32 Texture::MipChainSize() const
{
    uint32 last = m_mipCount - 1;
    return MipOffset(last) + MipSide(last) * MipSide(last);
}

uint32 Texture::CubeFaceSize() const
{
    return m_width / 3;
}

uint32 Texture::CubeLodCount() const
{
    return (uint32)glm::log2((float)CubeFaceSize()) + 1;
}

uint32 Texture::MaxCubeLod() const
{
    return glm::min((uint32)m_mipMapMax, CubeLodCount() - 1);
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

