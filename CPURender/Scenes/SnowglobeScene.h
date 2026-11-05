#ifndef SCENES_SNOWGLOBE_H
#define SCENES_SNOWGLOBE_H

#include "Scene.h"

#include "../Datas/Lights/LightData.hpp"
#include "../Maths/Transform.h"
#include "../Core/FrameBuffer.hpp"
#include "../Shaders/ShaderBase/Material.h"
#include "../Cubemap.h"
#include "../Shaders/Shaders.h"

class SnowglobeScene : public Scene
{
protected:
    void OnInit(Device* _pDevice) override;
    void OnHandleEvent(const SDL_Event& event) override;
    void OnUpdate(float dt) override;
    void OnDraw(Device* _pDevice) override;

private:
    void InitLightScene();
    void InitPassDesc(Device* _pDevice);
    void InitMaterials();
    void InitObjects();
    void CycleBlendMode(int _step);
    void IncreaseRefraction(float _value);

    Cubemap m_cubemap;

    Material m_floorMat;
    Material m_snowMat;
    Material m_chaletMat;
    Material m_glassMat;

    Material m_transparentMat;
    Material m_postProcMat;
    Material m_depthCopyMat;

    Material m_skyboxMat;
    bool m_toggleSkybox = false;
    Object* skyboxObject = nullptr;

    int m_blendIndex = Blending::ALPHA;
    PassDesc m_skyboxPass;
    PassDesc m_shadowPass;
    PassDesc m_mainPass;
    PassDesc m_particlesPass;
    PassDesc m_depthCopyPass;
    PassDesc m_refractionPass;
    PassDesc m_postProcPass;

    LitC_BP_Transparent_PS::MaterialCB m_transparentCB;

    LightData m_lightData;
};

#endif
