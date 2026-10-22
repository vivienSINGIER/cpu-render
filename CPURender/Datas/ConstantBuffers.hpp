#ifndef DATAS_CONSTANT_BUFFERS_HPP_DEFINED
#define DATAS_CONSTANT_BUFFERS_HPP_DEFINED

#include <glm/mat4x4.hpp>

struct PerPassCB
{
    glm::mat4x4 viewProj;
    glm::mat4x4 view;
    glm::mat4x4 proj;
    glm::mat4x4 invViewProj;

    glm::vec3 eyePos;
    float nearZ;
          
    float farZ;
    float totalTime;
    float deltaTime;
    float pad0;

    glm::vec2 renderTargetSize;
    glm::vec2 invRenderTargetSize;
};

struct PerObjectCB
{
    glm::mat4x4 model;
    glm::mat4x4 normalMatrix; // Transpose de l'inverse de la matrice model
};

#endif