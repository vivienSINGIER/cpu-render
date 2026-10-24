#include "Frustum.h"

#include "AABB.h"
#include <iostream>

Frustum::Frustum(bool _isGl)
{
    left = Plane(glm::vec3(1,0,0), -1.0f);
    right = Plane(glm::vec3(-1,0,0), -1.0f);
    top = Plane(glm::vec3(0, -1, 0), -1.0f);
    bottom = Plane(glm::vec3(0, 1, 0), -1.0f);
    farZ = Plane(glm::vec3(0, 0, -1), -1.0f);
    if (_isGl)
        nearZ = Plane(glm::vec3(0, 0, 1), -1.0f);
    else
        nearZ = Plane(glm::vec3(0, 0, 1), 0.0f);
}

Frustum::Frustum(glm::mat4x4 const& _view, glm::mat4x4 const& _proj, bool _isGl)
{
    auto GetColumn = [&](glm::mat4x4 vp, int _i)
        {
            glm::vec4 res = {};
            _i = _i % 4;

            for (int i = 0; i < 4; i++)
            {
                res[i] = vp[i][_i];
            }
            return res;
        };

    glm::mat4x4 vp = _proj * _view;
    
    glm::vec4 C0 = GetColumn(vp, 0);
    glm::vec4 C1 = GetColumn(vp, 1);
    glm::vec4 C2 = GetColumn(vp, 2);
    glm::vec4 C3 = GetColumn(vp, 3);

    glm::vec4 raw[6] =
    {
        C3 + C0, // Left
        C3 - C0, // Right
        C3 + C1, // Bottom
        C3 - C1, // Top
        C2,      // Near
        C3 - C2, // Far
    };

    if (_isGl)
        raw[4] += C3;

    for (int i = 0; i < 6; i++)
    {
        glm::vec3 normal(raw[i].x, raw[i].y, raw[i].z);
        float len = glm::length(normal);
        planes[i] = Plane(normal / len, -raw[i].w / len);
    }
}

Frustum::Frustum(glm::mat4x4 const& _vp, bool _isGl)
{
    auto GetColumn = [&](glm::mat4x4 vp, int _i)
        {
            glm::vec4 res = {};
            _i = _i % 4;

            for (int i = 0; i < 4; i++)
            {
                res[i] = vp[i][_i];
            }
            return res;
        };

    glm::vec4 C0 = GetColumn(_vp, 0);
    glm::vec4 C1 = GetColumn(_vp, 1);
    glm::vec4 C2 = GetColumn(_vp, 2);
    glm::vec4 C3 = GetColumn(_vp, 3);

    glm::vec4 raw[6] =
    {
        C3 + C0, // Left
        C3 - C0, // Right
        C3 + C1, // Bottom
        C3 - C1, // Top
        C2,      // Near
        C3 - C2, // Far
    };

    if (_isGl)
        raw[4] += C3;

    for (int i = 0; i < 6; i++)
    {
        glm::vec3 normal(raw[i].x, raw[i].y, raw[i].z);
        float len = glm::length(normal);
        planes[i] = Plane(normal / len, -raw[i].w / len);
    }
}

bool Frustum::Contains(glm::vec3 const& _p) const
{
    for (int i = 0; i < 6; i++)
    {
        if (planes[i].ClassifyPoint(_p) < 0)
            return false;
    }

    return true;
}

bool Frustum::Intersects(AABB const& _o) const
{
    for (int i = 0; i < 6; i++)
    {
        glm::vec3 n = planes[i].normal;
        
        glm::vec3 pVertex = {
            n.x >= 0 ? _o.max.x : _o.min.x,
            n.y >= 0 ? _o.max.y : _o.min.y,
            n.z >= 0 ? _o.max.z : _o.min.z
        };

        if (planes[i].ClassifyPoint(pVertex) < 0)
            return false;
    }

    return true;
}

void Frustum::PrintDebug() const
{
    for (int i = 0; i < 6; ++i)
    {
        const glm::vec3& n = planes[i].normal;

        std::cout << "Plane ";

        switch (i)
        {
        case 0:
            std::cout << "Left";
            break;

        case 1:
            std::cout << "Right";
            break;

        case 2:
            std::cout << "Bottom";
            break;

        case 3:
            std::cout << "Up";
            break;

        case 4:
            std::cout << "Near";
            break;

        case 5:
            std::cout << "Far";
            break;
        }

        std::cout << " : n(" << n.x << ", " << n.y << ", " << n.z << ")\n";
    }
}
