#pragma once
#include "Scene.h"

#include "../Datas/Lights/LightData.hpp"
#include "../Maths/Transform.h"
#include "../Core/FrameBuffer.hpp"
#include "../Shaders/ShaderBase/Material.h"

#include "../Datas/Particle.h"

// Blinn-Phong demo
//  INS / DEL   : next / previous material
//  HOME / END  : next / previous mesh
//  Arrows      : rotate the directional light (left/right = yaw, up/down = elevation)
class SceneBillingPhong : public Scene
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
    LightData m_lightData;

    std::vector<Material> m_materials;
    std::vector<Object*>  m_meshObjects;

    int m_materialIndex = 0;
    int m_meshIndex = 0;
    
    float m_lightYaw = 0.0f;
    float m_lightPitch = 0.0f;

    bool lights_dirty = false;
};
