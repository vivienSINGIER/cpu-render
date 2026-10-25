
#include "Camera.h"

glm::mat4x4 MakePerspective(float _fov, float _aspectRatio, float _near, float _far)
{
    float _00 = 1.0f / (_aspectRatio * std::tanf(_fov / 2));
    float _11 = 1.0f / std::tanf(_fov / 2);
    float _22 = -(_near + _far) / (_near - _far);
    float _23 = 1.0f;
    float _32 = -(2.0f * _far * _near) / (_far - _near);

    return {
        { _00, 0, 0, 0 },
        { 0, _11, 0, 0 },
        { 0, 0, _22, _23 },
        { 0, 0, _32, 0 },
    };
}

glm::mat4x4 MakeOrthographic(float _left, float _right, float _bottom, float _top, float _near, float _far)
{
    float _00 = 2.0f / (_right - _left);
    float _11 = 2.0f / (_top - _bottom);
    float _22 = -2.0f / (_far - _near);

    float _03 = -(_right + _left) / (_right - _left);
    float _13 = -(_top + _bottom) / (_top - _bottom);
    float _23 = -(_near + _far) / (_far - _near);
    float _33 = 1.0f;

    return {
        { _00, 0, 0, _03 },
        { 0, _11, 0, _13 },
        { 0, 0, _22, _23 },
        { 0, 0, 0, _33 }
    };
}

Camera::Camera()
{
    m_world = glm::identity<glm::mat4x4>();
    m_view = glm::identity<glm::mat4x4>();
    m_proj = glm::identity<glm::mat4x4>();
    m_viewProj = glm::identity<glm::mat4x4>();
}

Camera::~Camera()
{

}

void Camera::SetWorld(glm::mat4x4 const& _world)
{
    m_world = _world;
    UpdateMatrices();
}

void Camera::SetFov(float _fov)
{
    m_fov = _fov;
    m_proj = MakePerspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    m_shadowProj = MakePerspective(m_fov, m_aspectRatio, m_nearPlane, m_shadowDistance);
    UpdateMatrices();
}

void Camera::SetNearDistance(float _nearPlane)
{
    m_nearPlane = _nearPlane;
    m_proj = MakePerspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    m_shadowProj = MakePerspective(m_fov, m_aspectRatio, m_nearPlane, m_shadowDistance);
    UpdateMatrices();
}

void Camera::SetShadowDistance(float _shadowDistance)
{
    m_shadowDistance = _shadowDistance;
    m_shadowProj = MakePerspective(m_fov, m_aspectRatio, m_nearPlane, m_shadowDistance);
    UpdateMatrices();
}

void Camera::SetFarDistance(float _farPlane)
{
    m_farPlane = _farPlane;
    m_proj = MakePerspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    UpdateMatrices();
}

void Camera::SetAspectRatio(float _aspectRatio)
{
    m_aspectRatio = _aspectRatio;
    m_proj = MakePerspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    m_shadowProj = MakePerspective(m_fov, m_aspectRatio, m_nearPlane, m_shadowDistance);
    UpdateMatrices();
}

float Camera::GetFov() const
{
    return m_fov;
}

float Camera::GetNearDistance() const
{
    return m_nearPlane;
}

float Camera::GetFarDistance() const
{
    return m_farPlane;
}

float Camera::GetAspectRatio() const
{
    return m_aspectRatio;
}

glm::mat4x4 const& Camera::GetShadowProj() const
{
    return m_shadowProj;
}

glm::mat4x4 const& Camera::GetView() const
{
    return m_view;
}

void Camera::UpdateMatrices()
{
    m_view = glm::inverse(m_world);

    m_viewProj = m_proj * m_view;
    m_frustum = Frustum(m_viewProj, true);
    m_viewProj = glm::transpose(m_viewProj);

    m_pos = glm::vec3(m_world[3][0], m_world[3][1], m_world[3][2]);
}

Frustum const& Camera::GetFrustum()
{
    return m_frustum;
}

void Camera::FillData(PerPassCB* _passData)
{
    // TODO avoid redundant recomputation of the matrices (inverse)
    _passData->viewProj = m_viewProj;
    _passData->view = m_view;
    _passData->proj = m_proj;

    _passData->invViewProj = glm::inverse(m_viewProj);

    _passData->eyePos = m_pos;

    _passData->nearZ = m_nearPlane;
    _passData->farZ = m_farPlane;
}
