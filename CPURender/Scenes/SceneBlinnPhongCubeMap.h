#pragma once
#include "Scene.h"

#include "../Datas/Lights/LightData.hpp"
#include "../Maths/Transform.h"
#include "../Core/FrameBuffer.hpp"
#include "../Shaders/ShaderBase/Material.h"

#include "../Datas/Particle.h"
#include "../Cubemap.h"

// Blinn-Phong demo
//  INS / DEL   : next / previous material
//  HOME / END  : next / previous mesh
//  Arrows      : rotate the directional light (left/right = yaw, up/down = elevation)
class SceneBlinnPhongCubeMap : public Scene
{
protected:
    void OnInit(Device* _pDevice) override;
    void OnHandleEvent(const SDL_Event& event) override;
    void OnUpdate(float dt) override;
    void OnDraw(Device* _pDevice) override;

private:
    void SelectMaterial(int _index);
    void SelectMesh(int _index);
    MaterialData* GetOrCreateMaterial(int _index);

    void UpdateLightDirection();

    PassDesc m_mainPass;
    PassDesc m_shadowPass;
    PassDesc m_skyboxPass;

    Material m_skyboxMat;
    Object* skyboxObject = nullptr;

    LightData m_lightData;

    float m_metalness = 1.0f;
    float m_roughness = 0.5f;

    Material rock_mat;

    Cubemap m_cubemap;

    std::vector<Material> m_materials;
    std::vector<Object*>  m_meshObjects;

    int m_materialIndex = 0;
    int m_meshIndex = 0;

    float m_lightYaw = 0.0f;
    float m_lightPitch = 0.0f;

    bool lights_dirty = false;

    void SetSkybox(Cubemap _cubemap);
};