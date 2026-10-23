#include "Triangle.h"

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

glm::vec3 BarycentricWeight(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 weights = { 0.0f, 0.0f, 0.0f };
    glm::vec3 shiftedV2 = _tri.v2.positionH - _tri.v1.positionH;
    glm::vec3 shiftedV3 = _tri.v3.positionH - _tri.v1.positionH;
    glm::vec3 shiftedPoint = { _point.x - _tri.v1.positionH.x, _point.y - _tri.v1.positionH.y, 0.0f };

    float d = shiftedV2.x * shiftedV3.y - shiftedV3.x * shiftedV2.y;
    
    weights.y = (shiftedPoint.x * shiftedV3.y - shiftedPoint.y * shiftedV3.x) / d;
    weights.z = (shiftedPoint.y * shiftedV2.x - shiftedPoint.x * shiftedV2.y) / d;
    weights.x = 1 - weights.y - weights.z;
    
    // float alpha = weights.x / _tri.v1.positionH.z + weights.y / _tri.v2.positionH.z + weights.z / _tri.v3.positionH.z;
    //
    // glm::vec3 correctedW = {    weights.x / alpha * _tri.v1.positionH.z, 
    //                             weights.y / alpha * _tri.v2.positionH.z, 
    //                             weights.z / alpha * _tri.v3.positionH.z };

    return weights;
}

glm::vec3 BarycentricWeight(glm::vec3 _point, const Triangle& _tri)
{
    glm::vec3 a = _tri.v1.positionH;
    glm::vec3 b = _tri.v2.positionH;
    glm::vec3 c = _tri.v3.positionH;

    glm::vec3 u = b - a;
    glm::vec3 v = c - a;
    glm::vec3 n = glm::cross(u, v);

    glm::vec3 nA = glm::cross(c - b, _point - b);
    glm::vec3 nB = glm::cross(a - c, _point - c);

    float dA = glm::dot(nA, n) / glm::dot(n, n);
    float dB = glm::dot(nB, n) / glm::dot(n, n);
    float dC = 1 - dA - dB;

    glm::vec3 weights = { dA, dB, dC };
    
    // float alpha = weights.x / _tri.v1.positionH.z + weights.y / _tri.v2.positionH.z + weights.z / _tri.v3.positionH.z;
    //
    // glm::vec3 correctedW = {    weights.x / alpha * _tri.v1.positionH.z, 
    //                             weights.y / alpha * _tri.v2.positionH.z, 
    //                             weights.z / alpha * _tri.v3.positionH.z };
    
    return weights;
}

Vertex_OUT InterpolateVertex(glm::vec2 _point, const Triangle& _tri)
{
    glm::vec3 bary = BarycentricWeight(_point, _tri);
    Vertex_OUT vert;

    vert.positionH = { _point,
        glm::dot(bary, { _tri.v1.positionH.z, _tri.v2.positionH.z, _tri.v3.positionH.z }), 
        glm::dot(bary, { _tri.v1.positionH.w, _tri.v2.positionH.w, _tri.v3.positionH.w }) };

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

Vertex_OUT InterpolateVertex(glm::vec3 _point, const Triangle& _tri)
{
    glm::vec3 bary = BarycentricWeight(_point, _tri);
    Vertex_OUT vert;

    vert.positionH = { _point, glm::dot(bary, { _tri.v1.positionH.w, _tri.v2.positionH.w, _tri.v3.positionH.w }) };

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
    glm::vec3 bary = BarycentricWeight(_point, _tri);
    
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
