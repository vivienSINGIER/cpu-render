#ifndef AABB_H_DEFINED
#define AABB_H_DEFINED


#pragma once
#include <glm/matrix.hpp>

class Ray;
class Plane;

class AABB
{
public:
    glm::vec3 min;
    glm::vec3 max;
    
    AABB();
    AABB(glm::vec3 const& _min, glm::vec3 const& _max);
    
    static AABB FromMinMax(glm::vec3 const& _min, glm::vec3 const& _max);
    static AABB FromCenterExtent(glm::vec3 const& _center, glm::vec3 const& _extent);
    
    glm::vec3 Center() const;
    glm::vec3 Extent() const;
    float Volume() const;
    
    void SetCenter(glm::vec3 const& _center);
    void SetExtent(glm::vec3 const& _extent);
    
    void Merge(AABB const& _o);
    void Union(AABB const& _o);
    void Expand(float _scalar);
    
    void Transform(glm::mat4x4 const& _t);
    AABB Transformed(glm::mat4x4 const& _t) const;
    
    bool Contains(glm::vec3 const& _p)               const;
    bool Contains(AABB const& _o)                   const;
    bool Intersects(Ray const& _r, glm::vec3* _p)    const;
    bool Intersects(Plane const& _plane)            const;
    bool Intersects(AABB const& _o)                 const;
    
    static AABB Merge(AABB const& _a, AABB const& _b);
    static AABB Union(AABB const& _a, AABB const& _b);
    static AABB Expand(AABB const& _o, float _scalar);
    static AABB Transform(AABB const& _o, glm::mat4x4 const& _t);
};

#endif
