#ifndef SHADERS_PBR_HELPERS_HPP_DEFINED
#define SHADERS_PBR_HELPERS_HPP_DEFINED

#include "../../main.h"
#include "LightHelpers.hpp"

namespace Lighting
{
    struct PBRData
    {
        vec3 albedo;
        float metalness;
        
        vec3 f0;
        float roughness;
        
        float ambientOcc;
    };
    
    inline float DistributionGGX(vec3 N, vec3 H, float _roughness)
    {
        float a      = _roughness * _roughness;
        float a2     = a * a;
        float NdotH  = max(dot(N, H), 0.0f);
        float NdotH2 = NdotH * NdotH;
	
        float num   = a2;
        float denom = glm::max(NdotH2 * (a2 - 1.0f) + 1.0f, 0.0001f);
        denom = glm::pi<float>() * denom * denom;
	
        return num / denom;
    }

    inline float GeometrySchlickGGX(float NdotV, float roughness)
    {
        float r = roughness + 1.0f;
        float k = (r*r) / 8.0f;

        float num   = NdotV;
        float denom = NdotV * (1.0f - k) + k;
	
        return num / denom;
    }
    
    inline float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
    {
        float NdotV = max(dot(N, V), 0.0f);
        float NdotL = max(dot(N, L), 0.0f);
        float ggx2  = GeometrySchlickGGX(NdotV, roughness);
        float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
        return ggx1 * ggx2;
    }
    
    inline vec3 fresnelSchlick(float cosTheta, vec3 F0)
    {
        return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
    }  
    
    inline vec3 ComputePointPBR(Light _l, PBRData _mat, vec3 _pos, vec3 _normal, vec3 _toEye)
    {
        vec3 L = normalize(_l.position - _pos);
        vec3 H = normalize(_toEye + L);
        float distance    = length(_l.position - _pos);
        float attenuation = 1.0f / glm::max(distance * distance, 0.001f);
        vec3 radiance     = vec3(_l.color) * _l.strength * attenuation;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(_normal, H, _mat.roughness);        
        float G   = GeometrySmith(_normal, _toEye, L, _mat.roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, _toEye), 0.0f), _mat.f0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - _mat.metalness;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0f * max(dot(_normal, _toEye), 0.0f) * max(dot(_normal, L), 0.0f) + 0.0001f;
        vec3 specular     = numerator / denominator;  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(_normal, L), 0.0f);                
        return (kD * _mat.albedo / vec3(glm::pi<float>()) + specular) * radiance * NdotL;
    }
    
    inline vec3 ComputePBR(LightPassCB _lightData, PBRData _mat, vec3 _pos, vec3 _normal, vec3 _toEye)
    {
        vec3 result = vec4(0.0f);
    
        int i = 0;
    
        int limit = _lightData.dirLightCount;
    
        for (i = 0; i < limit; i++)
        {
            // TODO
        }
    
        limit += _lightData.pointLightCount;
    
        for (i = _lightData.dirLightCount; i < limit; i++)
        {
            result += ComputePointPBR(_lightData.lights[i], _mat, _pos, _normal, _toEye);
        }
    
        limit += _lightData.spotLightCount;
    
        for (i = _lightData.dirLightCount + _lightData.pointLightCount; i < limit; i++)
        {
            // TODO
        }
    
        return result;
    }
}

#endif
