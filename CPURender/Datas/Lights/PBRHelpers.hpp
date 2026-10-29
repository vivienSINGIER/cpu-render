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
    
    inline float RadicalInverse_VdC(uint32 bits)
    {
        bits = (bits << 16) | (bits >> 16);
        bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
        bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
        bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
        bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
        return (float)bits * 2.3283064365386963e-10f;
    }
    
    inline vec2 Hammersley(uint32 i, uint32 N)
    {
        return vec2((float)i / (float)N, RadicalInverse_VdC(i));
    }
    
    inline vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
    {
        float a = roughness * roughness;
        
        float phi = 2.0f * glm::pi<float>() * Xi.x;
        float cosTheta = glm::sqrt((1.0f - Xi.y) / (1.0f + (a*a - 1.0f) * Xi.y));
        float sinTheta = glm::sqrt(1.0f - cosTheta*cosTheta);
        
        vec3 H;
        H.x = cos(phi) * sinTheta;
        H.y = sin(phi) * sinTheta;
        H.z = cosTheta;
	
        // from tangent-space vector to world-space sample vector
        vec3 up        = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
        vec3 tangent   = normalize(cross(up, N));
        vec3 bitangent = cross(N, tangent);
	
        vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
        return normalize(sampleVec);
    }
    
    
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

    inline float GeometrySchlickGGX_IBL(float NdotV, float roughness)
    {
        float r = roughness + 1.0f;
        float k = (r*r) / 2.0f;

        float num   = NdotV;
        float denom = NdotV * (1.0f - k) + k;
	
        return num / denom;
    }
    
    inline float GeometrySmith_IBL(vec3 N, vec3 V, vec3 L, float roughness)
    {
        float NdotV = max(dot(N, V), 0.0f);
        float NdotL = max(dot(N, L), 0.0f);
        float ggx2  = GeometrySchlickGGX_IBL(NdotV, roughness);
        float ggx1  = GeometrySchlickGGX_IBL(NdotL, roughness);
	
        return ggx1 * ggx2;
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
    
    inline vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
    {
        return F0 + (glm::max(vec3(1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
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
        vec3 result = vec3(0.0f);
    
        int i = 0;
    
        int limit = _lightData.dirLightCount;
    
        // for (i = 0; i < limit; i++)
        // {
        //     // TODO
        // }
    
        limit += _lightData.pointLightCount;
    
        for (i = _lightData.dirLightCount; i < limit; i++)
        {
            result += ComputePointPBR(_lightData.lights[i], _mat, _pos, _normal, _toEye);
        }
    
        limit += _lightData.spotLightCount;
    
        // for (i = _lightData.dirLightCount + _lightData.pointLightCount; i < limit; i++)
        // {
        //     // TODO
        // }
    
        return result;
    }
}

#endif
