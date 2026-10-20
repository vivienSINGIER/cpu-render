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

uint32_t GetPointColor(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 bary = BarycentricWeight(_point, _tri);

    uint8_t colorArr[3] = { 0, 0, 0 };


    uint32_t color = 0;

    for (int i = 0; i < 3; i++)
    {
        colorArr[i] += bary.x * _tri.v1.color[i];
        colorArr[i] += bary.y * _tri.v2.color[i];
        colorArr[i] += bary.z * _tri.v3.color[i];
    }

    color = 255 << 24 | colorArr[0] << 16 | colorArr[1] << 8 | colorArr[2];

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
