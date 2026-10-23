#ifndef SHADERS_LIGHT_HELPERS_HPP_DEFINED
#define SHADERS_LIGHT_HELPERS_HPP_DEFINED

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/vec4.hpp>

#include "LightData.hpp"

struct BlinnPhongData
{
    glm::vec4 diffuse;
    float fresnelR0;
    
    float Shininess;
};

inline float CalcAttenuation(float _d, float _fallOfStart, float _fallOfEnd)
{
    return glm::clamp((_fallOfEnd - _d) / (_fallOfEnd - _fallOfStart), 0.0f, 1.0f );
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

inline glm::vec4 ComputeLighting(LightPassCB _lightData, BlinnPhongData _mat, glm::vec3 _pos, glm::vec3 _normal, glm::vec3 _toEye, glm::vec3 _shadowFactor)
{
    glm::vec3 result = glm::vec4(0.0f);
    
    int i = 0;
    
    int limit = _lightData.dirLightCount;
    
    for (i = 0; i < limit; i++)
    {
        result += ComputeDirectionalLight(_lightData.lights[i], _mat, _normal, _toEye);
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

#endif