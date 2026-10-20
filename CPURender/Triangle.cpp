#include "Triangle.h"

bool IsFront(const Triangle& _tri, bool _clockwise)
{
    glm::vec3 normal = glm::cross(_tri.v3.pos - _tri.v1.pos, _tri.v2.pos - _tri.v1.pos);

    if(_clockwise) return normal.z < 0;
    return normal.z > 0;
}

bool IsInTriangle(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 bar = BarycentricWeight(_point, _tri);
    return (bar.x <= 1.0f && bar.x >= 0.0f) && (bar.y <= 1.0f && bar.y >= 0.0f) && (bar.z <= 1.0f && bar.z >= 0.0f);
}

glm::vec3 BarycentricWeight(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 weights = { 0.0f, 0.0f, 0.0f };
    glm::vec3 shiftedV2 = _tri.v2.pos - _tri.v1.pos;
    glm::vec3 shiftedV3 = _tri.v3.pos - _tri.v1.pos;
    glm::vec3 shiftedPoint = { _point.x - _tri.v1.pos.x, _point.y - _tri.v1.pos.y, 0.0f };

    float d = shiftedV2.x * shiftedV3.y - shiftedV3.x * shiftedV2.y;
    
    weights.x = (shiftedPoint.x * (shiftedV2.y - shiftedV3.y) + shiftedPoint.y * (shiftedV3.x - shiftedV2.x) + shiftedV2.x * shiftedV3.y - shiftedV3.x * shiftedV2.y) / d;
    weights.y = (shiftedPoint.x * shiftedV3.y - shiftedPoint.y * shiftedV3.x) / d;
    weights.z = (shiftedPoint.y * shiftedV2.x - shiftedPoint.x * shiftedV2.y) / d;

    return weights;
}

Color Triangle::GetPointColor(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 bary = BarycentricWeight(_point);
    
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
    float minX = _tri.v1.pos.x;
    float maxX = _tri.v1.pos.x;
    float minY = _tri.v1.pos.y;
    float maxY = _tri.v1.pos.y;

    minX = _tri.v2.pos.x < minX ? _tri.v2.pos.x : minX;
    minX = _tri.v3.pos.x < minX ? _tri.v3.pos.x : minX;

    maxX = _tri.v2.pos.x > maxX ? _tri.v2.pos.x : maxX;
    maxX = _tri.v3.pos.x > maxX ? _tri.v3.pos.x : maxX;

    minY = _tri.v2.pos.y < minY ? _tri.v2.pos.y : minY;
    minY = _tri.v3.pos.y < minY ? _tri.v3.pos.y : minY;

    maxY = _tri.v2.pos.y > maxY ? _tri.v2.pos.y : maxY;
    maxY = _tri.v3.pos.y > maxY ? _tri.v3.pos.y : maxY;

    return glm::vec4(floor(minX), floor(minY), ceil(maxX), ceil(maxY));
}
