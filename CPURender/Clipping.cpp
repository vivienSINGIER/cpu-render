#include "Clipping.h"
namespace Clipping
{

    bool Intersecting(float& t, float& u, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
    {
        t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
        u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
        return (t <= 1 && t >= 0 && u <= 1 && u >= 0);
    }

    std::vector<uint8_t> GetBufferFrustumBitmask(std::vector<Vertex_OUT> _vBuffer)
    {
        std::vector<uint8_t> bufferFrustumBitmask;
        size_t vCount = _vBuffer.size();
        for (int i = 0; i < vCount; i++)
        {
            bufferFrustumBitmask.push_back(GetFrustumBitmask(_vBuffer[i]));
        }
        return bufferFrustumBitmask;
    }

    uint8_t GetBufferFrustumBitmaskOr(std::vector<uint8_t> _bufferFrustumBitmask)
    {
        uint8_t bitmask = 0;
        size_t vCount = _bufferFrustumBitmask.size();
        for (int i = 0; i < vCount; i++)
        {
            bitmask |= _bufferFrustumBitmask[i];
        }
        return bitmask;
    }

    uint8_t GetFrustumBitmask(Vertex_OUT _v)
    {
        uint8_t bitmask = 0;
        bitmask |= (!IsFrontOfPlane(_v, FrustumPlane::NEAR)  << 0);
        bitmask |= (!IsFrontOfPlane(_v, FrustumPlane::FAR)   << 1);
        bitmask |= (!IsFrontOfPlane(_v, FrustumPlane::LEFT)  << 2);
        bitmask |= (!IsFrontOfPlane(_v, FrustumPlane::RIGHT) << 3);
        bitmask |= (!IsFrontOfPlane(_v, FrustumPlane::UP)    << 4);
        bitmask |= (!IsFrontOfPlane(_v, FrustumPlane::DOWN)  << 5);

        return bitmask;
    }

    bool IsFrontOfPlane(Vertex_OUT _v, FrustumPlane _plane)
    {
        return (SignedDistanceToPlane(_v, _plane) > 0);
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