#ifndef RAY_H_DEFINED
#define RAY_H_DEFINED

#pragma once
#include <glm/vec3.hpp>

class Plane;
class AABB;

class Ray
{
public:
    glm::vec3 origin;
    glm::vec3 camDirection;
    float length;

    Ray(glm::vec3 _origin, glm::vec3 _direction, float _length);
    Ray(glm::vec3 _origin, glm::vec3 _direction);

    bool Contains(glm::vec3 const& _p) const;
    
    bool Intersects(Ray const& _ray, glm::vec3* _p = nullptr)          const;
    bool Intersects(Plane const& _plane, glm::vec3* _p = nullptr)      const;
    bool Intersects(AABB const& _aabb, glm::vec3* _p = nullptr)        const;
};

#endif
