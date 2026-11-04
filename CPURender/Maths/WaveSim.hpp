#ifndef MATH_WAVE_SIM_HPP_DEFINED
#define MATH_WAVE_SIM_HPP_DEFINED

#include "../main.h"

#include "../Datas/Texture.h"

#include <glm/vec2.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "../Shaders/ShaderBase/Shader.h"

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
    
    inline vec3 SeaHeight(Shader const& _sh, int _slot, vec2 _xz, float _time, int _iterCount, float _lod, SeaParams _prms)
    {
        const mat2 octave_m = mat2(1.6f, 1.2f, -1.2f, 1.6f);
        mat2 A = mat2(0.75f, 0.0f, 0.0f, 1.0f);     // uv.x *= 0.75 : uv = A * xz
        float freq = _prms.frequency, amp = _prms.height, choppy = _prms.choppy;
        vec3 res(0.0f);

        for (int i = 0; i < _iterCount; i++)
        {
            vec2 uv = A * _xz;
            
            for (float sgn : { 1.0f, -1.0f })
            {
                vec2 q = (uv + sgn * _time * _prms.speed) * freq;               // octave space
                vec4 s = _sh.SampleLod(_slot, q / P, _lod);                // .x = B, .yz = dB/dq
                float B = glm::max(s.x, 1e-4f);
                float h = glm::pow(B, choppy);
                vec2 dq = choppy * glm::pow(B, choppy - 1.0f) * vec2(s.y, s.z);
                res.x += h * amp;
                vec2 dxz = glm::transpose(A) * dq * freq;                // chain rule back to world
                res.y += dxz.x * amp;
                res.z += dxz.y * amp;
            }
            A = glm::transpose(octave_m) * A;  // "uv *= m" == m^T * uv
            freq *= 1.9f; amp *= 0.22f; choppy = glm::mix(choppy, 1.0f, 0.2f);
        }
        return res;
    }
    
    inline vec3 SeaHeight(Shader const& _sh, int _slot, vec2 _xz, float _time, int _iterCount, vec2 _uvDdx, vec2 _uvDdy, SeaParams _prms)
    {
        const mat2 octave_m = mat2(1.6f, 1.2f, -1.2f, 1.6f);
        mat2 A = mat2(0.75f, 0.0f, 0.0f, 1.0f);     // uv.x *= 0.75 : uv = A * xz
        float freq = _prms.frequency, amp = _prms.height, choppy = _prms.choppy;
        vec3 res(0.0f);

        for (int i = 0; i < _iterCount; i++)
        {
            vec2 uv = A * _xz;
            
            vec2 duv_dx = (freq / P) * (A * _uvDdx);
            vec2 duv_dy = (freq / P) * (A * _uvDdy);

            float rho = glm::max(glm::length(duv_dx), glm::length(duv_dy));   // texels per pixel / N
            float lod = glm::log2(rho * 512);
            lod = glm::clamp(lod, 0.0f, 6.0f); 
            float fade     = 1.0f - glm::smoothstep(6.0f - 2.0f, 6.0f, lod);
            float gradFade = 1.0f - glm::smoothstep(0.05f, 0.5f, rho);
            if (fade <= 0.0f) break;
            
            for (float sgn : { 1.0f, -1.0f })
            {
                vec2 q = (uv + sgn * _time * _prms.speed) * freq;               // octave space
                vec4 s = _sh.SampleLod(_slot, q / P, lod);                // .x = B, .yz = dB/dq
                float B = glm::max(s.x, 1e-4f);
                float h = glm::pow(B, choppy);
                vec2 dq = choppy * glm::pow(B, choppy - 1.0f) * vec2(s.y, s.z);
                vec2 dxz = glm::transpose(A) * dq * freq;                // chain rule back to world
                res.x += h * amp * fade;
                res.y += dxz.x * amp * gradFade;
                res.z += dxz.y * amp * gradFade;
            }
            A = glm::transpose(octave_m) * A;  // "uv *= m" == m^T * uv
            freq *= 1.9f; amp *= 0.22f; choppy = glm::mix(choppy, 1.0f, 0.2f);
        }
        return res;
    }
}

#endif