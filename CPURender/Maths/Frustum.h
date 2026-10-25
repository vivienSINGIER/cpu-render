#ifndef FRUSTUM_H_DEFINED
#define FRUSTUM_H_DEFINED

#include "Plane.h"

class AABB;

class Frustum
{
public:
    union 
    {
        Plane planes[6];

        struct
        {
            Plane left;
            Plane right;
            Plane bottom;
            Plane top;
            Plane nearZ;
            Plane farZ;
        };
        
    };
    
    Frustum(bool _isGl = false);
    Frustum(glm::mat4x4 const& _view, glm::mat4x4 const& _proj, bool _isGl = false);
    Frustum(glm::mat4x4 const& _viewProj, bool _isGl = false);

    bool Contains(glm::vec3 const& _p) const;
    bool Intersects(AABB const& _o) const;

    void PrintDebug() const;
};

#endif
