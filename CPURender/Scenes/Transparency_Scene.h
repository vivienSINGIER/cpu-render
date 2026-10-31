#ifndef SCENES_TRANSPARENCY_HPP
#define SCENES_TRANSPARENCY_HPP

#include "Scene.h"

#include "../Datas/Lights/LightData.hpp"
#include "../Maths/Transform.h"
#include "../Core/FrameBuffer.hpp"
#include "../Shaders/ShaderBase/Material.h"
#include "../Cubemap.h"

class Transparency_Scene : public Scene
{
protected:
    void OnInit(Device* _pDevice) override;
    void OnHandleEvent(const SDL_Event& event) override;
    void OnUpdate(float dt) override;
    void OnDraw(Device* _pDevice) override;

private:
    void InitLightScene();
    void CycleBlendMode(int _step);

    Material m_floorMat;
    Material m_opaqueMat;
    Material m_glassMat;
    Material m_tintedMat;
    Material m_metalMat;

    Material m_transparentMat;
    Material m_postProcMat;
    Material m_depthCopyMat;

    Material m_skyboxMat;
    bool m_toggleSkybox = false;
    Object* skyboxObject = nullptr;

    int m_blendIndex = Blending::ALPHA;
    PassDesc m_skyboxPass;
    PassDesc m_mainPass;
    PassDesc m_depthCopyPass;
    PassDesc m_refractionPass;
    PassDesc m_postProcPass;

    LightData m_lightData;
};

#endif
