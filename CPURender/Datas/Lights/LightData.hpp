#ifndef DATAS_LIGHT_DATA_HPP_DEFINED
#define DATAS_LIGHT_DATA_HPP_DEFINED

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define MAX_LIGHTS 16

enum class LightType
{
    Directional, Point, Spot
};

struct Light
{
    glm::vec3 strength;
    float falloffStart; // point/spot light only
    glm::vec3 direction; // directional/spot light only
    float falloffEnd; // point/spot light only
    glm::vec3 position; // point light only
    float spotPower; // spot light only
    
    glm::vec4 color;
};

struct LightDesc
{
    LightType type;
    Light light;
};

struct LightPassCB
{
    Light lights[MAX_LIGHTS];
    
    float ambientLight;
    int dirLightCount;
    int pointLightCount;
    int spotLightCount;
};

struct LightData
{
    LightDesc lights[MAX_LIGHTS];
    float ambient = 0.2f;
    int dirLightCount;
    int pointLightCount;
    int spotLightCount;
};

inline void FillLightPassCB(LightData _data, LightPassCB& _out)
{
    std::vector<Light> lights;
    std::vector<Light> dirLights, pointLights, spotLights;

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (_data.lights[i].type == LightType::Directional)
            dirLights.push_back(_data.lights[i].light);
        else if (_data.lights[i].type == LightType::Point)
            pointLights.push_back(_data.lights[i].light);
        else if (_data.lights[i].type == LightType::Spot)
            spotLights.push_back(_data.lights[i].light);
    }

    _out.dirLightCount   = (int)dirLights.size();
    _out.pointLightCount = (int)pointLights.size();
    _out.spotLightCount  = (int)spotLights.size();

    lights.clear();
    lights.insert(lights.end(), dirLights.begin(),   dirLights.end());
    lights.insert(lights.end(), pointLights.begin(), pointLights.end());
    lights.insert(lights.end(), spotLights.begin(),  spotLights.end());

    memset(_data.lights, 0, sizeof(Light) * 16);
    memcpy(_data.lights, lights.data(), sizeof(Light) * lights.size());
}

inline LightDesc MakeDirectionalLight(glm::vec3 direction, float _intensity, Color _color = Color::WHITE)
{
    LightDesc d;
    d.type = LightType::Directional;
    
    d.light.direction = direction;
    d.light.strength = glm::vec3(_intensity);
    
    d.light.falloffStart = 0.0f;
    d.light.falloffEnd = 1.0f;
    d.light.color = Color::ToVec4(_color);
    d.light.position = glm::vec3(0.0f);
    return d;
}

inline LightDesc MakePointLight(glm::vec3 _pos, float _intensity, glm::vec2 _fallOff, Color _color = Color::WHITE)
{
    LightDesc d;
    d.type = LightType::Point;
    
    d.light.direction = glm::vec3(0.0f);
    d.light.strength = glm::vec3(_intensity);
    
    d.light.falloffStart = _fallOff.x;
    d.light.falloffEnd = _fallOff.y;
    d.light.color = Color::ToVec4(_color);
    d.light.position = _pos;
    return d;
}

inline LightDesc MakeSpotLight(glm::vec3 _pos, glm::vec3 _dir, float _intensity, glm::vec2 _fallOff, Color _color = Color::WHITE)
{
    LightDesc d;
    d.type = LightType::Point;
    
    d.light.direction = _dir;
    d.light.strength = glm::vec3(_intensity);
    
    d.light.falloffStart = _fallOff.x;
    d.light.falloffEnd = _fallOff.y;
    d.light.color = Color::ToVec4(_color);
    d.light.position = _pos;
    return d;
}

#endif