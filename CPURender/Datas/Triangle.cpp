#include "Triangle.h"
#include <iostream>

bool IsFront(const Triangle& _tri, bool _clockwise)
{
    glm::mat3x3 mat = { glm::vec3({ _tri.v1.positionH.x, _tri.v1.positionH.y, _tri.v1.positionH.w }), 
        glm::vec3({ _tri.v2.positionH.x, _tri.v2.positionH.y, _tri.v2.positionH.w }), 
        glm::vec3({ _tri.v3.positionH.x, _tri.v3.positionH.y, _tri.v3.positionH.w }) };
    float value = glm::determinant(mat);

    if(_clockwise) return value > 0;
    return value < 0;
}

bool IsInTriangle(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 bar = BarycentricWeight(_point, _tri);

    return (bar.x <= 1.0f && bar.x >= 0.0f) && (bar.y <= 1.0f && bar.y >= 0.0f) && (bar.z <= 1.0f && bar.z >= 0.0f);
}

float GetAreaDouble(glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
    return abs(glm::determinant(glm::mat2x2({b - a, c - a})));
}

glm::vec3 BarycentricWeightPerspective(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 weights = BarycentricWeight(_point, _tri);

    float gamma = weights.x / _tri.v1.wPreDiv + weights.y / _tri.v2.wPreDiv + weights.z / _tri.v3.wPreDiv;
    glm::vec3 weightsCorrected = {
        weights.x / (gamma * _tri.v1.wPreDiv),
        weights.y / (gamma * _tri.v2.wPreDiv), 
        weights.z / (gamma * _tri.v3.wPreDiv) };

    return weightsCorrected;
}

glm::vec3 BarycentricWeight(glm::vec2 _point, const Triangle& _tri)
{
    // Shift everything by v1
    glm::vec3 shiftedV2 = _tri.v2.positionH - _tri.v1.positionH;
    glm::vec3 shiftedV3 = _tri.v3.positionH - _tri.v1.positionH;
    glm::vec3 shiftedPoint = { _point.x - _tri.v1.positionH.x, _point.y - _tri.v1.positionH.y, 0.0f };

    // Det of {v2, v3}
    float d = shiftedV2.x * shiftedV3.y - shiftedV3.x * shiftedV2.y;

    glm::vec3 bar;

    bar.y = (shiftedPoint.x * shiftedV3.y - shiftedPoint.y * shiftedV3.x) / d;
    bar.z = (shiftedPoint.y * shiftedV2.x - shiftedPoint.x * shiftedV2.y) / d;
    bar.x = 1 - bar.y - bar.z;
    return bar;
}

Vertex_OUT InterpolateVertex(Vertex_OUT _v1, Vertex_OUT _v2, float _t)
{
    Vertex_OUT vert;
    vert.positionH = _v1.positionH + (_v2.positionH - _v1.positionH) * _t;
     
    vert.color.r = _v1.color.r + (int)((_v2.color.r - _v1.color.r) * _t);
    vert.color.g = _v1.color.g + (int)((_v2.color.g - _v1.color.g) * _t);
    vert.color.b = _v1.color.b + (int)((_v2.color.b - _v1.color.b) * _t);
   
    vert.uv = _v1.uv + (_v2.uv - _v1.uv) * _t;
    vert.normal = _v1.normal + (_v2.normal - _v1.normal) * _t;
    vert.tangent = _v1.tangent + (_v2.tangent - _v1.tangent) * _t;

    return vert;
}

Vertex_OUT InterpolateVertex(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 bary = BarycentricWeightPerspective(_point, _tri);
    Vertex_OUT vert;

    vert.positionModel = {
        glm::dot(bary, { _tri.v1.positionModel.x, _tri.v2.positionModel.x, _tri.v3.positionModel.x }),
        glm::dot(bary, { _tri.v1.positionModel.y, _tri.v2.positionModel.y, _tri.v3.positionModel.y }),
        glm::dot(bary, { _tri.v1.positionModel.z, _tri.v2.positionModel.z, _tri.v3.positionModel.z }),
        1.0f
    };
    
    vert.positionH = { _point,
        glm::dot(bary, { _tri.v1.positionH.z, _tri.v2.positionH.z, _tri.v3.positionH.z }), 
        glm::dot(bary, { _tri.v1.positionH.w, _tri.v2.positionH.w, _tri.v3.positionH.w }) };

    vert.uv = { glm::dot(bary, { _tri.v1.uv.x, _tri.v2.uv.x, _tri.v3.uv.x }),
                glm::dot(bary, { _tri.v1.uv.y, _tri.v2.uv.y, _tri.v3.uv.y }) };

    vert.normal = { glm::dot(bary, { _tri.v1.normal.x, _tri.v2.normal.x, _tri.v3.normal.x }),
                    glm::dot(bary, { _tri.v1.normal.y, _tri.v2.normal.y, _tri.v3.normal.y }),
                    glm::dot(bary, { _tri.v1.normal.z, _tri.v2.normal.z, _tri.v3.normal.z }) };

    vert.color = _tri.v1.color * bary.x + _tri.v2.color * bary.y + _tri.v3.color * bary.z;
    
    vert.uv = { glm::dot(bary, { _tri.v1.uv.x, _tri.v2.uv.x, _tri.v3.uv.x }),
                glm::dot(bary, { _tri.v1.uv.y, _tri.v2.uv.y, _tri.v3.uv.y }) };

    vert.normal = { glm::dot(bary, { _tri.v1.normal.x, _tri.v2.normal.x, _tri.v3.normal.x }),
                    glm::dot(bary, { _tri.v1.normal.y, _tri.v2.normal.y, _tri.v3.normal.y }),
                    glm::dot(bary, { _tri.v1.normal.z, _tri.v2.normal.z, _tri.v3.normal.z }) };
    
    vert.tangent = { glm::dot(bary, { _tri.v1.tangent.x, _tri.v2.tangent.x, _tri.v3.tangent.x }),
                    glm::dot(bary, { _tri.v1.tangent.y, _tri.v2.tangent.y, _tri.v3.tangent.y }),
                    glm::dot(bary, { _tri.v1.tangent.z, _tri.v2.tangent.z, _tri.v3.tangent.z }) };
    
    return vert;
}

Color GetPointColor(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 bary = BarycentricWeightPerspective(_point, _tri);
    
    Color color = Color::BLACK;

    color.r += static_cast<uint8_t>(bary.x * static_cast<float>(_tri.v1.color.r)); 
    color.g += static_cast<uint8_t>(bary.x * static_cast<float>(_tri.v1.color.g)); 
    color.b += static_cast<uint8_t>(bary.x * static_cast<float>(_tri.v1.color.b));
    color.a += static_cast<uint8_t>(bary.x * static_cast<float>(_tri.v1.color.a));
    
    color.r += static_cast<uint8_t>(bary.y * static_cast<float>(_tri.v2.color.r));
    color.g += static_cast<uint8_t>(bary.y * static_cast<float>(_tri.v2.color.g));
    color.b += static_cast<uint8_t>(bary.y * static_cast<float>(_tri.v2.color.b));
    color.a += static_cast<uint8_t>(bary.y * static_cast<float>(_tri.v2.color.a));
    
    color.r += static_cast<uint8_t>(bary.z * static_cast<float>(_tri.v3.color.r));
    color.g += static_cast<uint8_t>(bary.z * static_cast<float>(_tri.v3.color.g));
    color.b += static_cast<uint8_t>(bary.z * static_cast<float>(_tri.v3.color.b));
    color.a += static_cast<uint8_t>(bary.z * static_cast<float>(_tri.v3.color.a));

    return color;
}

glm::vec4 GetBoundingBox(const Triangle& _tri)
{
    float minX = _tri.v1.positionH.x;
    float maxX = _tri.v1.positionH.x;
    float minY = _tri.v1.positionH.y;
    float maxY = _tri.v1.positionH.y;

    minX = _tri.v2.positionH.x < minX ? _tri.v2.positionH.x : minX;
    minX = _tri.v3.positionH.x < minX ? _tri.v3.positionH.x : minX;

    maxX = _tri.v2.positionH.x > maxX ? _tri.v2.positionH.x : maxX;
    maxX = _tri.v3.positionH.x > maxX ? _tri.v3.positionH.x : maxX;

    minY = _tri.v2.positionH.y < minY ? _tri.v2.positionH.y : minY;
    minY = _tri.v3.positionH.y < minY ? _tri.v3.positionH.y : minY;

    maxY = _tri.v2.positionH.y > maxY ? _tri.v2.positionH.y : maxY;
    maxY = _tri.v3.positionH.y > maxY ? _tri.v3.positionH.y : maxY;

    return glm::vec4(floor(minX), floor(minY), ceil(maxX), ceil(maxY));
}
