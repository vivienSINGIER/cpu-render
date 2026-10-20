#ifndef PLANE_H_DEFINED
#define PLANE_H_DEFINED

#pragma once
#include <glm/matrix.hpp>
#include <glm/gtc/constants.hpp>

class AABB;
class Ray;

class Plane
{
public:
    float distance;
    glm::vec3 normal;

    Plane();
    Plane(glm::vec3 const& _normal, float _dist);
    Plane(glm::vec3 const& _normal, glm::vec3 const& _pos);
    Plane(glm::vec3 const& _p1, glm::vec3 const& _p2, glm::vec3 const& _p3);

    int ClassifyPoint(glm::vec3 const& _p)       const;
    float DistanceToPoint(glm::vec3 const& _p)   const;
    
    bool Contains(glm::vec3 const& _pos)     const;
    bool Intersects(Plane const& _o)        const;
    bool Intersects(Ray const& _r)          const;
    bool Intersects(AABB const& _a)         const;
    
    static bool ThreeWayIntersect(Plane const& _o1, Plane const& _o2, Plane const& _o3, glm::vec3* _p);
};

#endif
