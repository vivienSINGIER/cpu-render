#include "SceneParticlesTest.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

#include "../Shaders/ComputeShaders/ParticleCS.hpp"
#include "../Factories/BufferFactory.h"

void SceneParticlesTest::OnInit(Device* _pDevice)
{
    m_lightData.lights[0] = MakeDirectionalLight({ 1.0f, -1.0f, 1.0f }, 0.75f, Color::WHITE);
    m_lightData.ambient = 0.2f;

    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_presentTarget = m_mainPass.fb.gBuffer[0];

    m_particlesPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_particlesPass.isCompute = true;

    _pDevice->SetLightData(m_lightData);

    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));

    Handle diff = TextureFactory::Instance()->CreateTexture("dark_rock_diffuse", "Assets/Textures/DarkRock/diffuse.png");
    Handle ambient = TextureFactory::Instance()->CreateTexture("dark_rock_ambient", "Assets/Textures/DarkRock/ambient.png");
    Handle normal = TextureFactory::Instance()->CreateTexture("dark_rock_normal", "Assets/Textures/DarkRock/normal.png");
    Handle roughness = TextureFactory::Instance()->CreateTexture("dark_rock_roughness", "Assets/Textures/DarkRock/roughness.png");
    Handle displacement = TextureFactory::Instance()->CreateTexture("dark_rock_displacement", "Assets/Textures/DarkRock/displacement.png");
    Handle metalness = TextureFactory::Instance()->CreateTexture("dark_rock_metalness", "Assets/Textures/DarkRock/metalness.png");

    LitT_BP_PS::MaterialCB cb = {
        {1.0f, 1.0f, 1.0f, 1.0f},
            Lighting::g_fresnelR0Templates[PLASTIC],
                0.5f, 0.02f, 0.8f
    };

    Mesh m = GeometryFactory::Instance()->BuildUVSphere(10, 10, true);
    Mesh cube = GeometryFactory::Instance()->BuildCube();

    m_mat = ShaderFactory::Instance()->CreateMaterial("LitT_BP");
    m_mat->SetParams<LitT_BP_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_diffuse, diff);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_ambient, ambient);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_normal, normal);

    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_roughness, roughness);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_displacement, displacement);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_metal, metalness);

    Object* o = CreateObject(DYNAMIC);
    o->mesh = m;
    o->mat = m_mat.get();
    o->transform.SetPosition(glm::vec3(0, 2, 0));

    Object* floor = CreateObject(DYNAMIC);
    floor->mesh = cube;
    floor->mat = m_mat.get();
    floor->transform.SetPosition(glm::vec3(0.0f, -2.f, 0.f));
    floor->transform.SetScale(glm::vec3(25.0f, 0.2f, 25.0f));

    m_uav_buffer = BufferFactory::Instance()->CreateUAVBuffer<UavParticle>(MAX_PARTICLES);
    m_CS_handle = ShaderFactory::Instance()->CreateComputeShader<ParticleCS>();

    m_uav_view = m_pDevice->m_ctx.uavHeap.GetView(m_uav_buffer);

    m_particleMat = ShaderFactory::Instance()->CreateComputeMaterial("Particle");
    m_particleMat->SetUAVBuffer(0, m_uav_buffer);

    ShaderFactory::Instance()->RegisterGroup<UnlitParticle_VS, UnlitParticle_PS>("UnlitParticle");
    m_unlitParticle = ShaderFactory::Instance()->CreateMaterial("UnlitParticle");
    m_unlitParticle->SetUAVBuffer(0, m_uav_buffer);

    BufferFactory::Instance()->Fill(m_uav_buffer, UavParticle());
    ParticleManager::Instance()->m_uavBuffer = m_uav_buffer;

    ParticleDesc desc2;
    desc2.amount = 250;
    desc2.spawnRadius = 8.f;
    desc2.particleSize = 0.025f;
    desc2.rotationSpeed = 0.f;
    desc2.floorDir = glm::vec3(-0.15f, -1, -0.15f);
    desc2.ceilDir = glm::vec3(0.15f, -0.5f, 0.15f);
    desc2.minSpeed = 1.f;
    desc2.maxSpeed = 15.f;
    desc2.beginColor = glm::vec4(1, 1, 1, 1);
    desc2.endColor = glm::vec4(0, 0, 0, 1);
    desc2.minLifeTime = 1.f;
    desc2.maxLifeTime = 1.f;

    rain = ParticleManager::Instance()->CreateEmitter({0, 5, 0}, desc2, 0.1f);

    //TODO CHANGE
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (int k = 0; k < MAX_PARTICLES; k++)
    {
        for (int v = 0; v < 6; v++)
        {
            Vertex vert = Vertex();
            vert.id = (size_t)(k*6 + v);

            vertices.push_back(vert);
        }

        indices.push_back(k*6);
        indices.push_back(k*6 + 2);
        indices.push_back(k*6 + 1);

        indices.push_back(k*6 + 3);
        indices.push_back(k*6 + 1);
        indices.push_back(k*6 + 2);
    }

    mesh_particle = GeometryFactory::Instance()->CreateMesh(vertices, indices);
}

void SceneParticlesTest::OnHandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_A))
    {
        ParticleDesc desc1;
        desc1.amount = 100;
        desc1.spawnRadius = 0.f;
        desc1.particleSize = 0.1f;
        desc1.rotationSpeed = 100.f;
        desc1.floorDir = glm::vec3(-1, -1, -1);
        desc1.ceilDir = glm::vec3(1, 1, 1);
        desc1.minSpeed = 0.25f;
        desc1.maxSpeed = 1.f;
        desc1.beginColor = glm::vec4(1, 0, 0, 1);
        desc1.endColor = glm::vec4(1, 1, 0, 1);
        desc1.minLifeTime = 3.f;
        desc1.maxLifeTime = 5.f;

        ParticleManager::Instance()->Spawn({0, 0, 0}, desc1);
    }
}

void SceneParticlesTest::OnUpdate(float dt)
{
    ParticleManager::Instance()->UpdateEmitter(rain, dt);
}

void SceneParticlesTest::OnDraw(Device* _pDevice)
{
    m_camera.FillData(&m_mainPass.passData);
    m_mainPass.passData.deltaTime = m_deltaTime;
    m_mainPass.passData.totalTime = m_totalTime;
   
    _pDevice->BeginPass(m_mainPass);
    DrawObjects(_pDevice, m_camera.GetFrustum());
    _pDevice->EndPass();

    _pDevice->BeginPass(m_particlesPass);

    size_t count = m_uav_view.count;
    _pDevice->Dispatch(m_particleMat.get(), count / ParticleCS::GROUP_SIZE + 1, 1, 1);

    Transform world = {};
    glm::vec3 zero = { 0, 0, 0 };
    world.SetPosition(zero);
    UnlitParticle_PS::ParticleColor cb =
    {
        glm::vec4(1, 1, 1, 1)
    };

    m_unlitParticle->SetParams<UnlitParticle_PS::ParticleColor>(UnlitParticle_PS::CB_SLOT_pcolor, cb);
    _pDevice->RegisterCommand(mesh_particle, world.GetMatrix(), m_unlitParticle.get());
  
    /*  for (int i = 0; i < count; i++)
    {
        const UavParticle& p = BufferFactory::Instance()->Read<UavParticle>(m_uav_buffer, i);

        if (p.lifeTime <= 0.f)
            continue;

        world.SetScale(p.size);
        world.SetRotationQuaternion(m_camTransform.GetRotation());
        world.AddYPR(glm::vec3(0, 0, p.rotationRad));

  

      
    }*/

    _pDevice->EndPass();
}
