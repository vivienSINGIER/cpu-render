#ifndef TRANSFORM_H_DEFINED
#define TRANSFORM_H_DEFINED

#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>

class Transform
{
public:
    enum LocalDirty : uint8_t
    {
        None            = 0,
        Position        = 1 << 0,
        RotationScale   = 1 << 1,
        RotationMatrix  = 1 << 2,
        Inverse         = 1 << 3,
        World           = Position | RotationScale,
        All             = World | Inverse | RotationMatrix
    };
    
    Transform();
    ~Transform() = default;

    Transform(Transform const& _o);
    Transform(Transform&& _o) noexcept;
    Transform& operator=(Transform const& _o);
    Transform& operator=(Transform&& _o) noexcept;
    
    void SetIdentity();
    
    void UpdateMatrix();
    void UpdateInvMatrix();
    
    bool IsWorldDirty()     const;
    bool IsInverseDirty()   const;
    
    glm::mat4x4 const& GetMatrix();
    glm::mat4x4 const& GetInvMatrix();
    
    glm::mat4x4 UpdateFromParent(glm::mat4x4 const& _p);
    
    void AddFlag(uint8_t _flag);
    void RemoveFlag(uint8_t _flag);
    bool GetDirtyState(uint8_t _flag) const;
    
    //////////////////// Pos //////////////////////////
    
    glm::vec3 const& GetPosition();
    
    void SetPosition(glm::vec3 const& _position);
    void Move(glm::vec3 const& _delta);
    void Move(glm::vec3 const& _dir, float _distance);

    ////////////////////// Scale //////////////////////
   
    glm::vec3 const& GetScale();
    
    void SetScale(glm::vec3 const& _scale);
    void SetScale(float _scale);
    void Scale(glm::vec3 const& _scale);
    void Scale(float _scale);

    ///////////////////// Rotation /////////////////////

    glm::quat const& GetRotation();
    glm::vec3 GetEulerAngles();
    
    glm::vec3 const& GetRight();
    glm::vec3 const& GetUp();
    glm::vec3 const& GetForward();

    glm::mat3x3 const& GetRotMatrix();

    void SetRotationMatrix(glm::mat3x3 const& _rotation);
    void SetRotationQuaternion(glm::quat const& _quat);

    void SetYPR(glm::vec3 const& _ypr);
    void AddYPR(glm::vec3 const& _ypr);
    
    void UpdateRotationFromQuaternion();
    void ResetRotation();

    ///////////////// Other ////////////////////////////
    
    void LookAt(glm::vec3 const& _target, glm::vec3 const& _up = glm::vec3(0.0f, 1.0f, 0.0f));
    void LookTo(glm::vec3 const& _dir, glm::vec3 const& _up = glm::vec3(0.0f, 1.0f, 0.0f));

private:
    glm::vec3 m_pos;
    glm::vec3 m_scale;

    glm::quat m_quat;
    glm::mat3x3 m_rotMatrix;
    
    glm::mat4x4 m_matrix;
    glm::mat4x4 m_invMatrix;

    uint8_t m_dirty;
};

#endif
