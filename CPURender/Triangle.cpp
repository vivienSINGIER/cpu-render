#include "Triangle.h"

bool Triangle::IsFront(bool _clockwise)
{
    glm::vec3 normal = glm::cross(v2.pos - v1.pos, v3.pos - v1.pos);

    if(_clockwise) return normal.z < 0;
    return normal.z > 0;
}

bool Triangle::IsInTriangle(glm::vec2 _point)
{
    glm::vec3 bar = BarycentricWeight(_point);
    return (bar.x <= 1.0f && bar.x >= 0.0f) && (bar.y <= 1.0f && bar.y >= 0.0f) && (bar.z <= 1.0f && bar.z >= 0.0f);
}

glm::vec3 Triangle::BarycentricWeight(glm::vec2 _point)
{
    glm::vec3 weights = { 0.0f, 0.0f, 0.0f };
    glm::vec3 shiftedV2 = v2.pos - v1.pos; 
    glm::vec3 shiftedV3 = v3.pos - v1.pos; 
    glm::vec3 shiftedPoint = { _point.x - v1.pos.x, _point.y - v1.pos.y, 0.0f };

    float d = shiftedV2.x * shiftedV3.y - shiftedV3.x * shiftedV2.y;
    
    weights.x = (shiftedPoint.x * (shiftedV2.y - shiftedV3.y) + shiftedPoint.y * (shiftedV3.x - shiftedV2.x) + shiftedV2.x * shiftedV3.y - shiftedV3.x * shiftedV2.y) / d;
    weights.y = (shiftedPoint.x * shiftedV3.y - shiftedPoint.y * shiftedV3.x) / d;
    weights.z = (shiftedPoint.y * shiftedV2.x - shiftedPoint.x * shiftedV2.y) / d;

    return weights;
}

Color Triangle::GetPointColor(glm::vec2 _point)
{
    glm::vec3 bary = BarycentricWeight(_point);
    
    Color color = Color::BLACK;

    color.r += static_cast<uint8_t>(bary.x * static_cast<float>(v1.color.r)); 
    color.g += static_cast<uint8_t>(bary.x * static_cast<float>(v1.color.g)); 
    color.b += static_cast<uint8_t>(bary.x * static_cast<float>(v1.color.b));
    color.a += static_cast<uint8_t>(bary.x * static_cast<float>(v1.color.a));
    
    color.r += static_cast<uint8_t>(bary.y * static_cast<float>(v2.color.r));
    color.g += static_cast<uint8_t>(bary.y * static_cast<float>(v2.color.g));
    color.b += static_cast<uint8_t>(bary.y * static_cast<float>(v2.color.b));
    color.a += static_cast<uint8_t>(bary.y * static_cast<float>(v2.color.a));
    
    color.r += static_cast<uint8_t>(bary.z * static_cast<float>(v3.color.r));
    color.g += static_cast<uint8_t>(bary.z * static_cast<float>(v3.color.g));
    color.b += static_cast<uint8_t>(bary.z * static_cast<float>(v3.color.b));
    color.a += static_cast<uint8_t>(bary.z * static_cast<float>(v3.color.a));

    return color;
}

glm::vec4 Triangle::GetBoundingBox()
{
    float minX = v1.pos.x;
    float maxX = v1.pos.x;
    float minY = v1.pos.y;
    float maxY = v1.pos.y;

    minX = v2.pos.x < minX ? v2.pos.x : minX;
    minX = v3.pos.x < minX ? v3.pos.x : minX;

    maxX = v2.pos.x > maxX ? v2.pos.x : maxX;
    maxX = v3.pos.x > maxX ? v3.pos.x : maxX;

    minY = v2.pos.y < minY ? v2.pos.y : minY;
    minY = v3.pos.y < minY ? v3.pos.y : minY;

    maxY = v2.pos.y > maxY ? v2.pos.y : maxY;
    maxY = v3.pos.y > maxY ? v3.pos.y : maxY;

    return glm::vec4(floor(minX), floor(minY), ceil(maxX), ceil(maxY));
}
