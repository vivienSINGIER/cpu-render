#include "Ray.h"

#include "AABB.h"
#include "Plane.h"

Ray::Ray(glm::vec3 _origin, glm::vec3 _direction, float _length)
{
    origin = _origin;
    camDirection = glm::normalize(_direction);
    length = _length;
}

Ray::Ray(glm::vec3 _origin, glm::vec3 _direction)
{
    origin = _origin;
    camDirection = glm::normalize(_direction);
    length = -1.0f;
}

bool Ray::Contains(glm::vec3 const& _p) const
{
    if (length == 0.0f) return false;
    
    glm::vec3 diff = _p - origin;
    glm::vec3 dir = glm::normalize(diff);

    if (glm::dot(camDirection, dir) != 1.0f ) return false;
    if ( length < 0.0f ) return true;

    float dist = glm::length(diff);

    return dist * dist < length * length;
}

bool Ray::Intersects(Ray const& _ray, glm::vec3* _p) const
{
    glm::vec3 w = glm::normalize(origin - _ray.origin);

    float a = glm::dot(w, camDirection);
    float b = 1.0f; // Assumes normalized dir1
    float c = glm::dot(camDirection, _ray.camDirection);
    float d = glm::dot(_ray.camDirection, w);
    float e = 1.0f; // Assumes normalized dir2
    float denom = 1.0f - c*c;

    if (std::abs(denom) < glm::epsilon<float>())
        return false;

    float dReciprocal = 1.0f / denom;
    float t = (d*c - a*e) * dReciprocal;
    float s = (d*b - a*c) * dReciprocal;

    bool valid1 = false;
    bool valid2 = false;

    if ( length < 0.0f )
        valid1 = t >= 0.0f;
    else
        valid1 = t <= length;
    
    if ( _ray.length < 0.0f )
        valid2 = s >= 0.0f;
    else
        valid2 = s <= _ray.length;

    if (valid1 && valid2)
    {
        if (_p != nullptr)
            *_p = origin + t * camDirection;
        
        return true;
    }

    return false;
}

bool Ray::Intersects(Plane const& _plane, glm::vec3* _p) const
{
    float denom = glm::dot(_plane.normal, camDirection);
    bool isParallel = std::abs(denom) <= glm::epsilon<float>();

    if (isParallel) return false;

    float t = ( _plane.distance - glm::dot(_plane.normal, origin) ) / denom;

    if (length < 0.0f && _p == nullptr)
        return true;

    if (t < 0.0f)
        return false;

    if (length >= 0.0f && t > length)
        return false;

    if (_p != nullptr)
        *_p = origin + t * camDirection;

    return true;
}

bool Ray::Intersects(AABB const& _aabb, glm::vec3* _p) const
{
    glm::vec3 min = _aabb.min;
    glm::vec3 max = _aabb.max;

    float tNear = - std::numeric_limits<float>::max();
    float tFar  = std::numeric_limits<float>::max();

    for (int i = 0; i < 3; i++)
    {
        float o = origin[i];
        float d = camDirection[i];
        float bMin = min[i];
        float bMax = max[i];

        if (std::abs(d) < glm::epsilon<float>())
        {
            if (o < bMin || o > bMax)
                return false;
        }
        else
        {
            float t0 = (bMin - o) / d;
            float t1 = (bMax - o) / d;

            // TODO Add Swap util method & use
            if (t0 > t1)
            {
                float temp = t0;
                t0 = t1;
                t1 = temp;
            }

            tNear = std::max(tNear, t0);
            tFar  = std::min(tFar, t1);

            if (tNear > tFar)
                return false;
        }
    }

    if (tFar < 0.0f)
        return false;
    if (tNear > length)
        return false;

    if (_p != nullptr)
    {
        if (tNear >= 0.0f)
            *_p = origin + tNear * camDirection;
        else
            *_p = origin + tFar * camDirection;
    }
    
    return true;
}