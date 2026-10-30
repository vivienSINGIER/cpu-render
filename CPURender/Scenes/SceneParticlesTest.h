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
    Material m_unlitParticle;

    PassDesc m_mainPass;
    PassDesc m_particlesPass;

    LightData m_lightData;

    Handle m_CS_handle;
    Material m_particleMat;

    Handle m_uav_buffer;
    UAVView m_uav_view;

    Mesh mesh_particle;


    Object* sphere = nullptr;
    ParticleEmitter fire;
    ParticleEmitter rain;
};

