#include "Transform.h"

Transform::Transform()
{
    SetIdentity();
    m_dirty = 0;
}

Transform::Transform(Transform const& _o) :
    m_pos(_o.m_pos), m_scale(_o.m_scale),
    m_quat(_o.m_quat), m_rotMatrix(_o.m_rotMatrix),
    m_matrix(_o.m_matrix),
    m_invMatrix(_o.m_invMatrix),
    m_dirty(_o.m_dirty)
{
    
}

Transform::Transform(Transform&& _o) noexcept :
    m_pos(_o.m_pos), m_scale(_o.m_scale),
    m_quat(_o.m_quat), m_rotMatrix(_o.m_rotMatrix),
    m_matrix(_o.m_matrix),
    m_invMatrix(_o.m_invMatrix),
    m_dirty(_o.m_dirty)
{
    
}

Transform& Transform::operator=(Transform const& _o) 
{
    if (this == &_o)
        return *this;

    m_pos = _o.m_pos;
    m_scale = _o.m_scale;
    m_quat = _o.m_quat;
    m_rotMatrix = _o.m_rotMatrix;
    m_matrix = _o.m_matrix;
    m_invMatrix = _o.m_invMatrix;
    m_dirty = _o.m_dirty;

    return *this;
}

Transform& Transform::operator=(Transform&& _o) noexcept
{
    if (this == &_o)
        return *this;

    m_pos = _o.m_pos;
    m_scale = _o.m_scale;
    m_quat = _o.m_quat;
    m_rotMatrix = _o.m_rotMatrix;
    m_matrix = _o.m_matrix;
    m_invMatrix = _o.m_invMatrix;
    m_dirty = _o.m_dirty;


    return *this;
}

void Transform::SetIdentity()
{
    m_pos = { 0.0f, 0.0f, 0.0f };
    m_scale = { 1.0f, 1.0f, 1.0f };
    m_quat = glm::identity<glm::quat>();

    m_matrix = glm::identity<glm::mat4x4>();
    m_invMatrix = glm::identity<glm::mat4x4>();

    ResetRotation();
}

void Transform::UpdateMatrix()
{
    if (GetDirtyState(World))
    {
        glm::mat4x4 rot = glm::mat4_cast(m_quat);

        m_matrix[0] = m_scale.x * rot[0];
        m_matrix[1] = m_scale.y * rot[1];
        m_matrix[2] = m_scale.z * rot[2];
        m_matrix[3] = glm::vec4(m_pos, 1);

        RemoveFlag(World);
    }
    
    if (GetDirtyState(RotationScale))
    {
        glm::mat3x3 rotScale = GetRotMatrix();
        for (int i = 0; i < 3; i++)
        {
            rotScale[i] *= m_scale[i];

            m_matrix[i] = glm::vec4(rotScale[i], 0);
        }
        
        RemoveFlag(RotationScale);
    }
    
    if (GetDirtyState(Position))
    {
        m_matrix[3] = glm::vec4(m_pos, 1);
        RemoveFlag(Position);   
    }
}

void Transform::UpdateInvMatrix()
{
    if (IsWorldDirty() == true)
        UpdateMatrix();

    m_invMatrix = glm::inverse(m_matrix);
    RemoveFlag(Inverse);
}

bool Transform::IsWorldDirty() const
{
   return GetDirtyState(RotationScale) || GetDirtyState(Position);
}

bool Transform::IsInverseDirty() const
{
    return GetDirtyState(Inverse);
}

glm::mat4x4 const& Transform::GetMatrix()
{
    if (IsWorldDirty() == true)
        UpdateMatrix();

    return m_matrix;
}

glm::mat4x4 const& Transform::GetInvMatrix()
{
    if (IsInverseDirty() == true)
        UpdateInvMatrix();

    return m_invMatrix;
}

glm::mat4x4 Transform::UpdateFromParent(glm::mat4x4 const& _p)
{
    return GetMatrix() * _p;
}

void Transform::AddFlag(uint8_t _flag)
{
    m_dirty |= _flag;
}

void Transform::RemoveFlag(uint8_t _flag)
{
    m_dirty &= ~_flag;
}

bool Transform::GetDirtyState(uint8_t _flag) const
{
    return (m_dirty & _flag) == _flag;   
}

#pragma region TRANSLATION

glm::vec3 const& Transform::GetPosition()
{
    return m_pos;
}

void Transform::SetPosition(glm::vec3 const& _position)
{
    m_pos = _position;
    AddFlag(Position | Inverse);
}

void Transform::Move(glm::vec3 const& _delta)
{
    m_pos += _delta;
    AddFlag(Position | Inverse);
}

void Transform::Move(glm::vec3 const& _dir, float _distance)
{
    m_pos += _dir * _distance;
    AddFlag(Position | Inverse);
}

#pragma endregion

#pragma region SCALE

glm::vec3 const& Transform::GetScale()
{
    return m_scale;
}

void Transform::SetScale(glm::vec3 const& _scale)
{
    m_scale = _scale;
    AddFlag(RotationScale | Inverse);
}

void Transform::SetScale(float _scale)
{
    m_scale = glm::vec3(_scale, _scale, _scale);
    AddFlag(RotationScale | Inverse);
}

void Transform::Scale(glm::vec3 const& _scale)
{
    m_scale *= _scale;
    AddFlag(RotationScale | Inverse);
}

void Transform::Scale(float _scale)
{
    m_scale *= _scale;
    AddFlag(RotationScale | Inverse);
}

#pragma endregion 

#pragma region ROTATION

glm::quat const& Transform::GetRotation()
{
    return m_quat;
}

glm::vec3 Transform::GetEulerAngles()
{
    return glm::eulerAngles(m_quat);
}

glm::vec3 const& Transform::GetRight()
{
    if ( GetDirtyState(RotationMatrix) == true )
        UpdateRotationFromQuaternion();

    return m_rotMatrix[0];
}

glm::vec3 const& Transform::GetUp()
{
    if ( GetDirtyState(RotationMatrix) == true )
        UpdateRotationFromQuaternion();

    return m_rotMatrix[1];
}

glm::vec3 const& Transform::GetForward()
{
    if ( GetDirtyState(RotationMatrix) == true )
        UpdateRotationFromQuaternion();

    return m_rotMatrix[2];
}

glm::mat3x3 const& Transform::GetRotMatrix()
{
    if ( GetDirtyState(RotationMatrix) == true )
        UpdateRotationFromQuaternion();

    return m_rotMatrix;
}

void Transform::SetRotationMatrix(glm::mat3x3 const& _rotation)
{
    m_rotMatrix = _rotation;
    m_quat = glm::quat_cast(m_rotMatrix);

    AddFlag(RotationScale | Inverse);
}

void Transform::SetRotationQuaternion(glm::quat const& _quat)
{
    m_quat = _quat;

    AddFlag(RotationMatrix | RotationScale | Inverse);
}

void Transform::SetYPR(glm::vec3 const& _ypr)
{
    m_quat = glm::quat(_ypr);
    AddFlag(RotationMatrix | RotationScale | Inverse);
}

void Transform::AddYPR(glm::vec3 const& _ypr)
{
    m_quat *= glm::quat(_ypr);
    AddFlag(RotationMatrix | RotationScale | Inverse);
}

void Transform::UpdateRotationFromQuaternion()
{
    m_rotMatrix = glm::mat3_cast(m_quat);
    RemoveFlag(RotationMatrix);
}

void Transform::ResetRotation()
{
    m_quat = glm::identity<glm::quat>();

    m_rotMatrix = glm::identity<glm::mat3x3>();;
}

#pragma endregion

void Transform::LookAt(glm::vec3 const& _target, glm::vec3 const& _up)
{
    glm::vec3 up = _up;
    glm::vec3 forward = glm::normalize(_target - m_pos);
   
    if (std::abs(glm::dot(forward, _up)) > 1.0f - glm::epsilon<float>())
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    up = glm::cross(forward, right);
    
    m_rotMatrix[0] = right;
    m_rotMatrix[1] = up;
    m_rotMatrix[2] = forward;
    m_quat = glm::quat_cast(m_rotMatrix);
    
    RemoveFlag(RotationMatrix);
    AddFlag(RotationScale | Inverse);
}

void Transform::LookTo(glm::vec3 const& _dir, glm::vec3 const& _up)
{
    glm::vec3 up = _up;
    glm::vec3 forward = glm::normalize(_dir);
    
    if (std::abs(glm::dot(forward, _up)) > 1.0f - glm::epsilon<float>())
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    up = glm::cross(forward, right);
    
    m_rotMatrix[0] = right;
    m_rotMatrix[1] = up;
    m_rotMatrix[2] = forward;
    m_quat = glm::quat_cast(m_rotMatrix);
    
    RemoveFlag(RotationMatrix);
    AddFlag(RotationScale | Inverse);
}
