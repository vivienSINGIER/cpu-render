#pragma once
#include "Scene.h"

#include "../Datas/Lights/LightData.hpp"
#include "../Maths/Transform.h"
#include "../Core/FrameBuffer.hpp"
#include "../Shaders/ShaderBase/Material.h"

#include "../Datas/Particle.h"

class SceneParticlesTest : public Scene
{
protected:
    void OnInit(Device* _pDevice) override;
    void OnHandleEvent(const SDL_Event& event) override;
    void OnUpdate(float dt) override;
    void OnDraw(Device* _pDevice) override;

private:
    Material m_mat;

    PassDesc m_mainPass;
    PassDesc m_particlesPass;

    LightData m_lightData;
    Material m_unlit;

    Material model_mat;
    Material hand_mat;

    SkinnedObject* obj = nullptr;
    ParticleEmitter* fire = nullptr;
};

