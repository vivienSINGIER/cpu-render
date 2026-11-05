#include "Triangle.h"
#include <iostream>

template <typename T>
inline T Lerp3(const T& a, const T& b, const T& c, const glm::vec3& w)
{
    return a * w.x + b * w.y + c * w.z;
}

bool IsFront(const Triangle& _tri, bool _clockwise)
{
    glm::mat3x3 mat = { glm::vec3({ _tri.v1.posH.x, _tri.v1.posH.y, _tri.v1.posH.w }), 
        glm::vec3({ _tri.v2.posH.x, _tri.v2.posH.y, _tri.v2.posH.w }), 
        glm::vec3({ _tri.v3.posH.x, _tri.v3.posH.y, _tri.v3.posH.w }) };
    float value = glm::determinant(mat);

    if(_clockwise) return value > 0;
    return value < 0;
}

bool IsInTriangle(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 bar = BarycentricWeight(_point, _tri);

    return (bar.x <= 1.0f && bar.x >= 0.0f) && (bar.y <= 1.0f && bar.y >= 0.0f) && (bar.z <= 1.0f && bar.z >= 0.0f);
}

bool IsInTriangle(glm::vec3 _bary)
{
    return (_bary.x <= 1.0f && _bary.x >= 0.0f) && (_bary.y <= 1.0f && _bary.y >= 0.0f) && (_bary.z <= 1.0f && _bary.z >= 0.0f);
}

float GetAreaDouble(glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
    return abs(glm::determinant(glm::mat2x2({b - a, c - a})));
}

glm::vec3 BarycentricWeightPerspective(const glm::vec3& _bary, float _w1, float _w2, float _w3)
{
    glm:: vec3 b = { _bary.x * _w1, _bary.y * _w2, _bary.z * _w3 };
    return b / (b.x + b.y + b.z);
}

glm::vec3  BarycentricWeight(const glm::vec2& _point, const Triangle& _tri)
{
    // Shift everything by v1
    glm::vec3 shiftedV2 = _tri.v2.posH - _tri.v1.posH;
    glm::vec3 shiftedV3 = _tri.v3.posH - _tri.v1.posH;
    glm::vec3 shiftedPoint = { _point.x - _tri.v1.posH.x, _point.y - _tri.v1.posH.y, 0.0f };

    // Det of {v2, v3}
    float invDet = 1.0f / (shiftedV2.x * shiftedV3.y - shiftedV3.x * shiftedV2.y);

    glm::vec3 bary;
    bary.y = (shiftedPoint.x * shiftedV3.y - shiftedPoint.y * shiftedV3.x) * invDet;
    bary.z = (shiftedPoint.y * shiftedV2.x - shiftedPoint.x * shiftedV2.y) * invDet;
    bary.x = 1 - bary.y - bary.z;

    return bary;
}

void BarycentricOffsets(const Triangle& _tri, glm::vec3& _offsetX, glm::vec3& _offsetY)
{
    glm::vec3 e2 = _tri.v2.posH - _tri.v1.posH;
    glm::vec3 e3 = _tri.v3.posH - _tri.v1.posH;

    // Det of {v2, v3}
    float invDet = 1.0f / (e2.x * e3.y - e3.x * e2.y);
    
    _offsetX = { (e2.y - e3.y) * invDet,  e3.y * invDet, -e2.y * invDet };
    _offsetY = { (e3.x - e2.x) * invDet, -e3.x * invDet,  e2.x * invDet };
}

Vertex_OUT InterpolateVertex(const Vertex_OUT& _v1, const Vertex_OUT& _v2, float _t)
{
    Vertex_OUT vert;
    vert.posH = _v1.posH + (_v2.posH - _v1.posH) * _t;
    vert.posModel = _v1.posModel + (_v2.posModel - _v1.posModel) * _t;
    vert.posLightSpace = _v1.posLightSpace + (_v2.posLightSpace - _v1.posLightSpace) * _t;
     
    vert.color.r = _v1.color.r + ((_v2.color.r - _v1.color.r) * _t);
    vert.color.g = _v1.color.g + ((_v2.color.g - _v1.color.g) * _t);
    vert.color.b = _v1.color.b + ((_v2.color.b - _v1.color.b) * _t);
   
    vert.uv = _v1.uv + (_v2.uv - _v1.uv) * _t;
    vert.normal = _v1.normal + (_v2.normal - _v1.normal) * _t;
    vert.tangent = _v1.tangent + (_v2.tangent - _v1.tangent) * _t;

    return vert;
}

// Does not interpolate PositionH!
// This must be set before using InterpolatePositionH, allowing to precompute the depth-buffer test
void InterpolateVertex(Vertex_OUT& _vertOut, glm::vec2 _point, const Triangle& _tri, glm::vec3 _bary)
{
    _vertOut.posModel = glm::vec4(glm::vec3(Lerp3(_tri.v1.posModel, _tri.v2.posModel, _tri.v3.posModel, _bary)), 1.0f);
    _vertOut.posLightSpace = glm::vec4(Lerp3(_tri.v1.posLightSpace, _tri.v2.posLightSpace, _tri.v3.posLightSpace, _bary));

    _vertOut.uv = Lerp3(_tri.v1.uv, _tri.v2.uv, _tri.v3.uv, _bary);
    _vertOut.normal = Lerp3(_tri.v1.normal, _tri.v2.normal, _tri.v3.normal, _bary);
    _vertOut.color = Lerp3(_tri.v1.color, _tri.v2.color, _tri.v3.color, _bary);
    _vertOut.tangent = Lerp3(_tri.v1.tangent, _tri.v2.tangent, _tri.v3.tangent, _bary);
}

glm::vec2 InterpolateUV(const Triangle& _tri, glm::vec3 _bary)
{
    return Lerp3(_tri.v1.uv, _tri.v2.uv, _tri.v3.uv, _bary);
}

glm::vec4 InterpolatePositionH(glm::vec2 _point, const Triangle& _tri, glm::vec3 _bary)
{
    return { _point, glm::dot(_bary, { _tri.v1.posH.z, _tri.v2.posH.z, _tri.v3.posH.z }), 1 };
}

glm::vec4 GetBoundingBox(const Triangle& _tri)
{
    float minX = glm::min(_tri.v1.posH.x, glm::min(_tri.v2.posH.x, _tri.v3.posH.x));
    float minY = glm::min(_tri.v1.posH.y, glm::min(_tri.v2.posH.y, _tri.v3.posH.y));
    float maxX = glm::max(_tri.v1.posH.x, glm::max(_tri.v2.posH.x, _tri.v3.posH.x));
    float maxY = glm::max(_tri.v1.posH.y, glm::max(_tri.v2.posH.y, _tri.v3.posH.y));

    return glm::vec4(floor(minX), floor(minY), ceil(maxX), ceil(maxY));
}
