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

uint32_t Triangle::GetPointColor(glm::vec2 _point)
{
    glm::vec3 bary = BarycentricWeight(_point);

    uint8_t colorArr[3] = { 0, 0, 0 };


    uint32_t color = 0;

    for (int i = 0; i < 3; i++)
    {
        colorArr[i] += bary.x * v1.color[i];
        colorArr[i] += bary.y * v2.color[i];
        colorArr[i] += bary.z * v3.color[i];
    }

    color = 255 << 24 | colorArr[0] << 16 | colorArr[1] << 8 | colorArr[2];

    return color;
}
