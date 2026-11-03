#ifndef MATH_WAVE_SIM_HPP_DEFINED
#define MATH_WAVE_SIM_HPP_DEFINED

#include "../main.h"

#include "../Datas/Texture.h"

#include <glm/vec2.hpp>
#include <glm/ext/scalar_constants.hpp>

namespace WaveSim
{
    constexpr float P = 2.0f * glm::pi<float>() * 8.0f;
    constexpr int N = 50;
    
    inline float Hash12(glm::ivec2 _c)
    {
        _c = ((_c % N) + N) % N; // wrap to keep it tileable
        uvec2 q = uvec2(ivec2(_c)) * uvec2(1597334677u, 3812015801u);
        uint32 n = (q.x ^ q.y) * 1597334677u;
        return (float)n * (1.0f / 4294967295.0f);
    }
    
    inline float Noise(vec2 _p)
    {
        _p *= (float)N / P;
        vec2 i = floor(_p);
        vec2 f = fract(_p);
        vec2 u = f * f * (3.0f - 2.0f * f);
        return -1.0f + 2.0f * mix(mix(Hash12(i + vec2(0.0f, 0.0f)), Hash12(i + vec2(1.0f, 0.0f)), u.x),
                                mix(Hash12(i + vec2(0.0f, 1.0f)), Hash12(i + vec2(1.0f, 1.0f)), u.x), u.y);
    }
    
    inline float SeaBase(vec2 _x)
    {
        _x += Noise(_x);
        vec2 wv = 1.0f - abs(sin(_x));
        vec2 swv = abs(cos(_x));
        wv = mix(wv, swv, wv);
        return 1.0f - glm::pow(wv.x * wv.y, 0.65f);
    }
    
    inline void BakeSeaBase(Texture* _out)
    {
        const uint32 S = 512;
        std::vector<float> b(S * S);
        
#pragma omp parallel for
        for (int y = 0; y < (int)S; y++)
        {
            for (int x = 0; x < (int)S; x++)
            {
                b[y * S + x] = SeaBase(vec2((float)x + 0.5f, (float)y + 0.5f) * P / (float)S);
            }
        }
        
        const float texel = P / (float)S;
        
        for (uint32 y = 0; y < S; y++)
        {
            for (uint32 x = 0; x < S; x++)
            {
                auto at = [&](uint32 i, uint32 j) { return b[((j + S) % S) * S + (i + S) % S]; };
                float dx = (at(x + 1, y) - at(x - 1, y)) / (2.0f * texel);
                float dy = (at(x, y + 1) - at(x, y - 1)) / 2.0f;
                _out->Store(x, y, vec4(b[y * S + x], dx, dy, 0.0f));
            }
        }
        
        _out->GenerateMips();
    }
    
    struct SeaParams
    {
        float frequency = 0.6f;
        float height = 0.4f;
        float choppy = 4.0f;
        float speed = 0.8f;
    };
    
    vec3 SeaHeight()
}

#endif