#ifndef SCENES_PBR_HPP
#define SCENES_PBR_HPP

#include "Scene.h"

#include "../Datas/Lights/LightData.hpp"
#include "../Maths/Transform.h"
#include "../Core/FrameBuffer.hpp"
#include "../Shaders/ShaderBase/Material.h"
#include "../Cubemap.h"

class PBR_Scene : public Scene
{
protected:
    void OnInit(Device* _pDevice) override;
    void OnHandleEvent(const SDL_Event& event) override;
    void OnUpdate(float dt) override;
    void OnDraw(Device* _pDevice) override;
    
    void SetSkybox(Cubemap _cubemap);

private:
    void InitLightScene();
    void UpdateMaterial();

    Object* ref = nullptr;

    Material m_mat;
    Material m_skyboxMat;
    float m_metalness = 1.0f;
    float m_roughness = 0.5f;

    bool toggleSkybox = true;
    Object* skyboxObject = nullptr;
    
    bool togglePostProc = false;
    PassDesc m_skyboxPass;
    PassDesc m_shadowPass;
    PassDesc m_mainPass;
    PassDesc m_postProcPass;
	
    LightData m_lightData;
};

#endif
