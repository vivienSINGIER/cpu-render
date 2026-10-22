#include "Clipping.h"
namespace Clipping
{

    bool Intersecting(float& t, float& u, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
    {
        t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
        u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
        return (t <= 1 && t >= 0 && u <= 1 && u >= 0);
    }


    //std::pair<Vertex_OUT, SideMask> FindIntersectionWithClip(Vertex_OUT v1, Vertex_OUT v2)
    //{
    //    ////v1:
    //    //float x1 = glm::vec2(v1.positionH).x;
    //    //float y1 = glm::vec2(v1.positionH).y;

    //    //// v2:
    //    //float x2 = glm::vec2(v2.positionH).x;
    //    //float y2 = glm::vec2(v2.positionH).y;

    //    ///*glm::vec4 left = { viewport.x, viewport.y, viewport.x, viewport.w };
    //    //glm::vec4 up = { viewport.x, viewport.w, viewport.z, viewport.w };
    //    //glm::vec4 right = { viewport.z, viewport.y, viewport.z, viewport.w };
    //    //glm::vec4 down = { viewport.x, viewport.y, viewport.z, viewport.y };*/

    //    //SideMask intersectingSide = NONE;

    //    //float t, u;
    //    //if (x1 < viewport.x) // Left
    //    //{
    //    //    float x3 = left.x;
    //    //    float y3 = left.y;
    //    //    float x4 = left.z;
    //    //    float y4 = left.w;

    //    //    if (Intersecting(t, u, x1, y1, x2, y2, x3, y3, x4, y4)) intersectingSide = LEFT;
    //    //}
    //    //if (x1 > viewport.z && intersectingSide == NONE) // Right
    //    //{
    //    //    float x3 = right.x;
    //    //    float y3 = right.y;
    //    //    float x4 = right.z;
    //    //    float y4 = right.w;

    //    //    if (Intersecting(t, u, x1, y1, x2, y2, x3, y3, x4, y4)) intersectingSide = RIGHT;
    //    //}
    //    //if (y1 < viewport.y && intersectingSide == NONE) // Down
    //    //{
    //    //    float x3 = down.x;
    //    //    float y3 = down.y;
    //    //    float x4 = down.z;
    //    //    float y4 = down.w;

    //    //    if (Intersecting(t, u, x1, y1, x2, y2, x3, y3, x4, y4)) intersectingSide = DOWN;
    //    //}
    //    //if (y1 > viewport.w && intersectingSide == NONE) // Up
    //    //{
    //    //    float x3 = up.x;
    //    //    float y3 = up.y;
    //    //    float x4 = up.z;
    //    //    float y4 = up.w;

    //    //    if (Intersecting(t, u, x1, y1, x2, y2, x3, y3, x4, y4)) intersectingSide = UP;
    //    //}

    //    //if (intersectingSide == NONE) return { Vertex_OUT(), NONE};

    //    //Vertex_OUT intersection;
    //    //intersection.positionH = { x1 + (x2 - x1) * t, y1 + (y2 - y1) * t, 0, 1 };
    //    //intersection.positionH.z = v1.positionH.z * (1 - t) + v2.positionH.z * t;
    //    //intersection.positionH.w = v1.positionH.w * (1 - t) + v2.positionH.w * t;
    //    //intersection.color = v1.color * (1 - t) + v2.color * t;


    //    //return { intersection, intersectingSide };
    //}

    uint8_t GetFrustumBitmask(Vertex_OUT _v)
    {
        uint8_t bitmask = 0;
        bitmask |= ((SignedDistanceToPlane(_v, FrustumPlane::NEAR)  < 0) << 0);
        bitmask |= ((SignedDistanceToPlane(_v, FrustumPlane::FAR)   < 0) << 1);
        bitmask |= ((SignedDistanceToPlane(_v, FrustumPlane::LEFT)  < 0) << 2);
        bitmask |= ((SignedDistanceToPlane(_v, FrustumPlane::RIGHT) < 0) << 3);
        bitmask |= ((SignedDistanceToPlane(_v, FrustumPlane::UP)    < 0) << 4);
        bitmask |= ((SignedDistanceToPlane(_v, FrustumPlane::DOWN)  < 0) << 5);

        return bitmask;
    }

    float SignedDistanceToPlane(Vertex_OUT _v, FrustumPlane _side)
    {
        switch (_side)
        {
        case NEAR:  return _v.positionH.w + _v.positionH.z;
        case FAR:   return _v.positionH.w - _v.positionH.z;
        case DOWN:  return _v.positionH.w + _v.positionH.y;
        case UP:    return _v.positionH.w - _v.positionH.y;
        case LEFT:  return _v.positionH.w + _v.positionH.x;
        case RIGHT: return _v.positionH.w - _v.positionH.x;
        default: return 0.0f;
        }
    }

    float GetPlaneIntersection(Vertex_OUT _v1, Vertex_OUT _v2, FrustumPlane _plane)
    {
        float dist1 = abs(Clipping::SignedDistanceToPlane(_v1, _plane));
        float dist2 = abs(Clipping::SignedDistanceToPlane(_v2, _plane));

        return dist1 / (dist1 + dist2);
    }

}