#include "Plane.h"

#include "AABB.h"
#include "Ray.h"

Plane::Plane()
{
    normal = glm::vec3(0.0f, 1.0f, 0.0f);
    distance = 0.0f;
}

Plane::Plane(glm::vec3 const& _normal, float _dist)
{
    normal = glm::normalize(_normal);
    distance = _dist;
}

Plane::Plane(glm::vec3 const& _normal, glm::vec3 const& _pos)
{
    normal = glm::normalize(_normal);
    distance = glm::dot(normal, _pos);
}

Plane::Plane(glm::vec3 const& _p1, glm::vec3 const& _p2, glm::vec3 const& _p3)
{
    glm::vec3 a = _p2 - _p1;
    glm::vec3 b = _p3 - _p1;

    normal = glm::normalize(glm::cross(a, b));
    distance = glm::dot(normal, _p1);
}

int Plane::ClassifyPoint(glm::vec3 const& _p) const
{
    float dot = glm::dot(normal, _p) - distance;
    
    if (std::abs(dot) < glm::epsilon<float>())
        return 0;
    
    return (dot < 0) ? -1 : (dot > 0) ? 1 : 0;
}

float Plane::DistanceToPoint(glm::vec3 const& _p) const
{
    return glm::dot(normal, _p) - distance;
}

bool Plane::Contains(glm::vec3 const& _pos) const
{
    glm::vec3 o = normal * distance;
    glm::vec3 v = _pos - o;

    return glm::dot(o, v) == 0.0f;
}

bool Plane::Intersects(Plane const& _o) const
{
    return std::abs(glm::dot(normal, _o.normal)) != 1.0f;
}

bool Plane::Intersects(Ray const& _r) const
{
    return _r.Intersects(*this);
}

bool Plane::Intersects(AABB const& _a) const
{
    int minPos = ClassifyPoint(_a.min);
    int maxPos = ClassifyPoint(_a.max);

    if (minPos == 0 || maxPos == 0)
        return true;

    return minPos != maxPos;
}

bool Plane::ThreeWayIntersect(Plane const& _o1, Plane const& _o2, Plane const& _o3, glm::vec3* _p)
{
    glm::vec3 n1 = _o1.normal;
    glm::vec3 n2 = _o2.normal;
    glm::vec3 n3 = _o3.normal;
    
    float denom = glm::dot(n1, glm::cross(n2, n3));
    if (std::abs(denom) < glm::epsilon<float>())
        return false;
    
    float denomRec = 1.0f / denom;
    
    glm::vec3 numerator =
            _o1.distance * glm::cross(n2, n3) +
            _o2.distance * glm::cross(n3, n1) + 
            _o3.distance * glm::cross(n1, n2);
    
    *_p = numerator * denomRec;
    return true;
}




 

