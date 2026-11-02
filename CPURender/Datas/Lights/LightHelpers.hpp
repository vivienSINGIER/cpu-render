#ifndef SHADERS_LIGHT_HELPERS_HPP_DEFINED
#define SHADERS_LIGHT_HELPERS_HPP_DEFINED

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "LightData.hpp"
#include "../../Maths/Frustum.h"

namespace Lighting
{
    enum FresnelTemplates : int
    {
        WATER, PLASTIC, GLASS, CRYSTAL, DIAMOND, 
        COUNT  
    };
    
    static std::array<glm::vec3, FresnelTemplates::COUNT> g_fresnelR0Templates(
    {
        {0.02f, 0.02f, 0.02f}, // WATER
        { 0.04f, 0.04f, 0.04f}, // PLASTIC
        { 0.08f, 0.08f, 0.08f}, // GLASS
        {0.11, 0.11, 0.11}, // CRYSTAL
        {0.17, 0.17, 0.17}, // DIAMOND
    });
    
    struct BlinnPhongData
    {
        glm::vec4 diffuse;
        glm::vec3 fresnelR0;
    
        float Shininess;
    };

    inline float Luminance(glm::vec3 _c) { return glm::dot(_c, vec3(0.2126f, 0.7152f, 0.0722f)); }
    
    inline float CalcAttenuation(float _d, float _fallOfStart, float _fallOfEnd)
    {
        return (_fallOfEnd - _d) / (_fallOfEnd - _fallOfStart);
    } 

    inline glm::vec3 SchlickFresnel(glm::vec3 _r0, glm::vec3 _normal, glm::vec3 _lightVec)
    {
        float cosIncidentAngle = glm::clamp(glm::dot(_normal, _lightVec), 0.0f, 1.0f);
    
        float f0 = 1.0f - cosIncidentAngle;
        glm::vec3 reflectPercent = _r0 + (1.0f - _r0) * (f0 * f0 * f0 * f0);
    
        return reflectPercent;
    }

    inline glm::vec3 BlinnPhong(  glm::vec3 _lightStrength, glm::vec3 _lightVec, 
                        glm::vec3 _normal, glm::vec3 _toEye, BlinnPhongData _mat)
    {
        const float m = _mat.Shininess * 256.0f;
        glm::vec3 halfVec = normalize(_toEye + _lightVec);
    
        float roughnessFactor = (m + 8.0f) * pow(glm::max(dot(halfVec, _normal), 0.0f), m) / 8.0f;
        glm::vec3 fresnelFactor = SchlickFresnel(glm::vec3(_mat.fresnelR0), halfVec, _lightVec);
    
        glm::vec3 specAlbedo = fresnelFactor*roughnessFactor;
        specAlbedo = specAlbedo / (specAlbedo + 1.0f);
    
        return (glm::vec3(_mat.diffuse) + specAlbedo) * _lightStrength;
    }

    inline glm::vec3 ComputeDirectionalLight(Light _l, BlinnPhongData _mat, glm::vec3 _normal, glm::vec3 _toEye)
    {
        glm::vec3 lightVec = -_l.direction;
    
        float ndot1 = glm::max(dot(lightVec, _normal), 0.0f);
        glm::vec3 lightStrength = _l.strength * ndot1;
    
        return BlinnPhong(lightStrength, lightVec, _normal, _toEye, _mat) * glm::vec3(_l.color);
    }

    inline glm::vec3 ComputePointLight(Light _l, BlinnPhongData _mat, glm::vec3 _pos, glm::vec3 _normal, glm::vec3 _toEye)
    {
        glm::vec3 lightVec = _l.position - _pos;
    
        float d = length(lightVec);
        if (d > _l.falloffEnd)
            return glm::vec3(0.0f);
    
        lightVec /= d;
    
        float ndotl = glm::max(dot(lightVec, _normal), 0.0f);
        glm::vec3 lightStrength = _l.strength * ndotl;
    
        float att = CalcAttenuation(d, _l.falloffStart, _l.falloffEnd);
        lightStrength *= att;
    
        return BlinnPhong(lightStrength, lightVec, _normal, _toEye, _mat) * glm::vec3(_l.color);
    }

    inline glm::vec3 ComputeSpotLight(Light _l, BlinnPhongData _mat, glm::vec3 _pos, glm::vec3 _normal, glm::vec3 _toEye)
    {
        glm::vec3 lightVec = _l.position - _pos;
    
        float d = length(lightVec);
        if (d > _l.falloffEnd)
            return glm::vec3(0.0f);
    
        lightVec /= d;
    
        float ndot1 = glm::max(glm::dot(lightVec, _normal), 0.0f);
        glm::vec3 lightStrength = _l.strength * ndot1;
    
        float att = CalcAttenuation(d, _l.falloffStart, _l.falloffEnd);
        lightStrength *= att;
    
        float spotFactor = glm::pow(glm::max(glm::dot(-lightVec, _l.direction), 0.0f), _l.spotPower);
        lightStrength *= spotFactor;
    
        return BlinnPhong(lightStrength, lightVec, _normal, _toEye, _mat) * glm::vec3(_l.color);
    }

    inline glm::vec4 ComputeLighting(LightPassCB _lightData, BlinnPhongData _mat, glm::vec3 _pos, glm::vec3 _normal, glm::vec3 _toEye, float _shadowFactor)
    {
        glm::vec3 result = glm::vec4(0.0f);
    
        int i = 0;
    
        int limit = _lightData.dirLightCount;
    
        for (i = 0; i < limit; i++)
        {
            result += ComputeDirectionalLight(_lightData.lights[i], _mat, _normal, _toEye);
            if (i == 0)
                result *= (1.0f - _shadowFactor);
        }
    
        limit += _lightData.pointLightCount;
    
        for (i = _lightData.dirLightCount; i < limit; i++)
        {
            result += ComputePointLight(_lightData.lights[i], _mat, _pos, _normal, _toEye);
        }
    
        limit += _lightData.spotLightCount;
    
        for (i = _lightData.dirLightCount + _lightData.pointLightCount; i < limit; i++)
        {
            result += ComputeSpotLight(_lightData.lights[i], _mat, _pos, _normal, _toEye);
        }
    
        return glm::vec4(result, 0.0f);
    }
    
    inline glm::mat4x4 ComputeDirLightViewProj(
        glm::mat4x4 const& _camView,
        glm::mat4x4 const& _camProjShadow,
        glm::vec3 _lightDir,
        uint32_t _shadowMapSize,
        Frustum _out,
        float _zExtend)     // Extra depth towards the light for off screen caster
    {
        glm::mat4x4 invVP = glm::inverse(_camProjShadow * _camView);
        glm::vec3 corners[8]; // camera frustum corners in world space
        int i = 0;
        for (int x = 0; x < 2; x++)
            for (int y = 0; y < 2; y++)
                for (int z = 0; z < 2; z++)
                {
                    glm::vec4 p = invVP * glm::vec4(x * 2.0f - 1.0f, y * 2.0f - 1.0f, z * 2.0f - 1.0f, 1.0f);
                    corners[i++] = vec3(p) / p.w;
                }
        
        glm::vec3 center(0.0f);
        for (auto& c : corners)
            center += c;
        center /= 8.0f;
        
        float radius = 0.0f;
        for (auto& c : corners)
            radius = glm::max(radius, glm::length(c - center));
        
        _lightDir = glm::normalize(_lightDir);
        glm::vec3 up = glm::abs(_lightDir.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 eye = center - _lightDir * (radius + _zExtend);
        glm::mat4x4 lightView = glm::lookAtLH(eye, center, up);
        
        mat4x4 lightProj = glm::orthoLH_NO(-radius, radius, -radius, radius, 0.0f, 2.0f * radius + _zExtend);
        
        glm::mat4x4 viewProj = lightProj * lightView;
        glm::vec4 origin = viewProj * glm::vec4(0, 0, 0, 1);
        origin *= (float)_shadowMapSize * 0.5f;
        glm::vec4 rounded = round(origin);
        vec4 offset = (rounded - origin) * (2.0f / (float)_shadowMapSize);
        lightProj[3][0] += offset.x;
        lightProj[3][1] += offset.y;
        
        _out = Frustum(lightView, lightProj);
        return glm::transpose(lightProj * lightView);
    }
}
    
#endif