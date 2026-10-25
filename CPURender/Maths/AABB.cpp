#include "AABB.h"

#include "Ray.h"
#include "Plane.h"

AABB AABB::FromMinMax(glm::vec3 const& _min, glm::vec3 const& _max)
{
    return AABB(_min, _max);
}

AABB AABB::FromCenterExtent(glm::vec3 const& _center, glm::vec3 const& _extent)
{
    return AABB(_center - _extent, _center + _extent);
}

glm::vec3 AABB::Center() const
{
    return (max + min) / 2.0f;
}

glm::vec3 AABB::Extent() const
{
    return (max - min) / 2.0f;
}

float AABB::Volume() const
{
    glm::vec3 l = Extent() * 2.0f;
    
    return l.x * l.y * l.z;
}

void AABB::SetCenter(glm::vec3 const& _center)
{
    glm::vec3 extent = Extent();
    
    min = _center - extent;
    max = _center + extent;
}

void AABB::SetExtent(glm::vec3 const& _extent)
{
    glm::vec3 center = Center();
    
    min = center - _extent;
    max = center + _extent;
}

void AABB::Merge(AABB const& _o)
{
    glm::vec3 c = (Center() + _o.Center()) / 2.0f;
    glm::vec3 extent = (Extent() + _o.Extent()) / 2.0f;
    
    min = c - extent;
    max = c + extent;
}

void AABB::Union(AABB const& _o)
{
    min = glm::min(min, _o.min);
    max = glm::max(max, _o.max);
}

void AABB::Expand(float _scalar)
{
    glm::vec3 extent = Extent() * _scalar;
    glm::vec3 c = Center();
    
    min = c - extent;
    max = c + extent;
}

void AABB::Transform(glm::mat4x4 const& _t)
{
    glm::vec3 newMin = { _t[3][0], _t[3][1], _t[3][2] };
    glm::vec3 newMax = newMin;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            float coeff = _t[i][j];
            float minContrib = coeff * (coeff > 0.f ? min[i] : max[i]);
            float maxContrib = coeff * (coeff > 0.f ? max[i] : min[i]);
            newMin[j] += minContrib;
            newMax[j] += maxContrib;
        }
    }

    min = newMin;
    max = newMax;
}

AABB AABB::Transformed(glm::mat4x4 const& _t) const
{
    glm::vec3 newMin = { _t[3][0], _t[3][1], _t[3][2] };
    glm::vec3 newMax = newMin;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            float coeff = _t[i][j];
            float minContrib = coeff * (coeff > 0.f ? min[i] : max[i]);
            float maxContrib = coeff * (coeff > 0.f ? max[i] : min[i]);
            newMin[j] += minContrib;
            newMax[j] += maxContrib;
        }
    }

    return FromMinMax(newMin, newMax);
}

bool AABB::Contains(glm::vec3 const& _p) const
{
    bool validX = min.x <= _p.x && _p.x <= max.x;
    bool validY = min.y <= _p.y && _p.y <= max.y;
    bool validZ = min.z <= _p.z && _p.z <= max.z;
    
    return validX && validY && validZ;
}

bool AABB::Contains(AABB const& _o) const
{
    bool validX = min.x <= _o.min.x && max.x >= _o.max.x;
    bool validY = min.y <= _o.min.y && max.y >= _o.max.y;
    bool validZ = min.z <= _o.min.z && max.z >= _o.max.z;
    
    return validX && validY && validZ;
}

bool AABB::Intersects(Ray const& _r, glm::vec3* _p) const
{
    return _r.Intersects(*this, _p);
}

bool AABB::Intersects(Plane const& _plane) const
{
    return _plane.Intersects(*this);
}

bool AABB::Intersects(AABB const& _o) const
{
    glm::vec3 c = Center();
    glm::vec3 oC = _o.Center();
    
    glm::vec3 d = glm::abs(c - oC);

    glm::vec3 extent = Extent();
    glm::vec3 oExtent = _o.Extent();
    
    bool validX = d.x <= extent.x + oExtent.x;
    bool validY = d.y <= extent.y + oExtent.y;
    bool validZ = d.z <= extent.z + oExtent.z;
    
    return validX && validY && validZ;
}

AABB AABB::Merge(AABB const& _a, AABB const& _b)
{
    glm::vec3 c = (_a.Center() + _b.Center()) / 2.0f;
    glm::vec3 extent = (_a.Extent() + _b.Extent()) / 2.0f;
    
    return FromCenterExtent(c, extent);
}

AABB AABB::Union(AABB const& _a, AABB const& _b)
{
    return FromMinMax(glm::min(_a.min, _b.min), glm::max(_a.max, _b.max));
}

AABB AABB::Expand(AABB const& _o, float _scalar)
{
    glm::vec3 extent = _o.Extent() * _scalar;
    glm::vec3 c = _o.Center();
    
    return FromCenterExtent(c, extent);
}

AABB AABB::Transform(AABB const& _o, glm::mat4x4 const& _t)
{
    AABB res;
    
    glm::vec3 newMin = { _t[3][0], _t[3][1], _t[3][2] };
    glm::vec3 newMax = newMin;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            float coeff = _t[i][j];
            float minContrib = coeff * (coeff > 0.f ? _o.min[i] : _o.max[i]);
            float maxContrib = coeff * (coeff > 0.f ? _o.max[i] : _o.min[i]);
            newMin[j] += minContrib;
            newMax[j] += maxContrib;
        }
    }

    res.min = newMin;
    res.max = newMax;

    return res;
}

AABB::AABB()
{
    min = glm::vec3(-1.f, -1.f, -1.f);
    max = glm::vec3(1.f, 1.f, 1.f);
}

AABB::AABB(glm::vec3 const& _min, glm::vec3 const& _max)
{
    min = _min;
    max = _max;
}


