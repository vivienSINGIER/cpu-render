#ifndef SCENES_TEXTURED_BP_HPP
#define SCENES_TEXTURED_BP_HPP

#include "Scene.h"

#include "../Datas/Lights/LightData.hpp"
#include "../Maths/Transform.h"
#include "../Core/FrameBuffer.hpp"
#include "../Shaders/ShaderBase/Material.h"
#include "../Cubemap.h"

class TexturedBP_Scene : public Scene
{
protected:
    void OnInit(Device* _pDevice) override;
    void OnHandleEvent(const SDL_Event& event) override;
    void OnUpdate(float dt) override;
    void OnDraw(Device* _pDevice) override;

    void SetSkybox(Cubemap _cubemap);

private:
    void InitLightScene();

    Object* ref = nullptr;

    Material m_mat;
    Material m_skyboxMat;
    Material m_gammaCorrectMat;

    bool toggleSkybox = false;
    Object* skyboxObject = nullptr;

    bool togglePostProc = false;
    PassDesc m_skyboxPass;
    PassDesc m_shadowPass;
    PassDesc m_mainPass;
    PassDesc m_postProcPass;
	
    LightData m_lightData;
};

#endif
