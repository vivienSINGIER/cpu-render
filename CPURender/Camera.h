#ifndef CAMERA_H_DEFINED
#define CAMERA_H_DEFINED

#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Datas/ConstantBuffers.hpp"
#include "Maths/Frustum.h"

class Camera
{
public:
    Camera();
    ~Camera();

    void SetWorld(glm::mat4x4 const& _world);
    void SetFov(float _fov);
    void SetNearDistance(float _nearPlane);
    void SetFarDistance(float _farPlane);
    void SetAspectRatio(float _aspectRatio);

    float GetFov() const;
    float GetNearDistance() const;
    float GetFarDistance() const;
    float GetAspectRatio() const;

    void UpdateMatrices();
    Frustum const& GetFrustum();

private:
    glm::mat4x4 m_world;
    glm::mat4x4 m_view;
    glm::mat4x4 m_proj;
    glm::mat4x4 m_viewProj;

    glm::vec3 m_pos = { 0.0f, 0.0f, 0.0f };

    float m_fov = 0.25f * glm::pi<float>();
    float m_nearPlane = 1.0f;
    float m_farPlane = 1000.0f;
    float m_aspectRatio = 0.0f;

    Frustum m_frustum;

    void FillData(PerPassCB* _passData);

    friend class Device;
    friend class Engine;
};

#endif



