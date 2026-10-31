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

    Handle diff = TextureFactory::Instance()->CreateTexture("dark_rock_diffuse", "Assets/Obj/Plane/PaletteBaseColor_6.png");
    Handle ambient = TextureFactory::Instance()->CreateTexture("dark_rock_ambient", "Assets/Obj/Plane/nimrod_bottom_col_5.png");
    Handle normal = TextureFactory::Instance()->CreateTexture("dark_rock_normal", "Assets/Obj/Plane/nimrod_bottom_nrm_4.png");
    Handle roughness = TextureFactory::Instance()->CreateTexture("dark_rock_roughness", "Assets/Obj/Plane/PaletteMetallicRoughness.png");
   // Handle displacement = TextureFactory::Instance()->CreateTexture("dark_rock_displacement", "Assets/Textures/DarkRock/displacement.png");
    Handle metalness = TextureFactory::Instance()->CreateTexture("dark_rock_metalness", "Assets/Obj/Plane/PaletteMetallicRoughness2.png");

    LitT_BP_PS::MaterialCB cb = {
        {1.0f, 1.0f, 1.0f, 1.0f},
            Lighting::g_fresnelR0Templates[PLASTIC],
                0.5f, 0.02f, 0.8f
    };

    UnlitParticle_PS::ParticleColor color = { glm::vec4(1, 1, 1, 1) };
    UnlitC_PS::UnlitColor unlit_color = { glm::vec4(0, 1, 1, 1) };

    m_uav_buffer = BufferFactory::Instance()->CreateUAVBuffer<UavParticle>(MAX_PARTICLES);
    m_CS_handle = ShaderFactory::Instance()->CreateComputeShader<ParticleCS>();

    m_uav_view = m_pDevice->m_ctx.uavHeap.GetView(m_uav_buffer);

    m_particleMat = ShaderFactory::Instance()->CreateComputeMaterial("Particle");
    m_particleMat->SetUAVBuffer(0, m_uav_buffer);

    ShaderFactory::Instance()->RegisterGroup<UnlitParticle_VS, UnlitParticle_PS>("UnlitParticle");
    m_unlitParticle = ShaderFactory::Instance()->CreateMaterial("UnlitParticle");
    m_unlitParticle->SetUAVBuffer(0, m_uav_buffer);
    m_unlitParticle->SetParams<UnlitParticle_PS::ParticleColor>(UnlitParticle_PS::CB_SLOT_pcolor, color);

    m_unlit = ShaderFactory::Instance()->CreateMaterial("UnlitC");
    m_unlit->SetParams<UnlitC_PS::UnlitColor>(UnlitC_PS::CB_SLOT_mat, unlit_color);

    m_mat = ShaderFactory::Instance()->CreateMaterial("LitT_BP");
    m_mat->SetParams<LitT_BP_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_diffuse, diff);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_ambient, ambient);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_normal, normal);

    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_roughness, roughness);
    //m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_displacement, displacement);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_metal, metalness);

    Mesh m = GeometryFactory::Instance()->LoadMesh("Assets/Obj/Plane/plane.obj");

    obj = CreateObject(DYNAMIC);
    obj->mesh = m; // GeometryFactory::Instance()->BuildCube();
    obj->mat = m_mat.get();
    obj->transform.SetPosition(glm::vec3(-1.5f, 1.5f, -1.5f));
    obj->transform.SetScale(0.1f);

    Mesh cube = GeometryFactory::Instance()->BuildCube();
    Object* floor = CreateObject(DYNAMIC);
    floor->mesh = cube;
    floor->mat = m_unlit.get();
    floor->transform.SetPosition(glm::vec3(0.0f, -2.f, 0.f));
    floor->transform.SetScale(glm::vec3(25.0f, 0.2f, 25.0f));

    BufferFactory::Instance()->Fill(m_uav_buffer, UavParticle());
    ParticleManager::Instance()->m_uavBuffer = m_uav_buffer;

    ParticleDesc fireDesc;
    fireDesc.amount = 100;
    fireDesc.spawnRadius = 0.9f;
    fireDesc.particleBeginSize = 0.15f;
    fireDesc.particleEndSize = 0.f;

    fireDesc.rotationSpeed = 10.f;
    fireDesc.floorDir = glm::vec3(-0.25f, 0.5f, -0.25f);
    fireDesc.ceilDir = glm::vec3(0.25f, 1, 0.25f);
    fireDesc.minSpeed = 0.33f;
    fireDesc.maxSpeed = 3.f;
    fireDesc.beginColor = glm::vec4(1, 0, 0, 1);
    fireDesc.endColor = glm::vec4(1, 1, 0, 1);
    fireDesc.minLifeTime = 1.f;
    fireDesc.maxLifeTime = 3.f;

    fire = ParticleManager::Instance()->CreateEmitter({ 0, 2, 0 }, fireDesc, 0.2f);

    ParticleDesc rainDesc;
    rainDesc.amount = 0;
    rainDesc.spawnRadius = 12.5f;
    rainDesc.particleBeginSize = 0.025f;
    rainDesc.particleEndSize = 0.025f;
    rainDesc.rotationSpeed = 0.f;
    rainDesc.floorDir = glm::vec3(-0.25f, -1, -0.25f);
    rainDesc.ceilDir = glm::vec3(0.25f, -0.5f, 0.25f);
    rainDesc.minSpeed = 15.f;
    rainDesc.maxSpeed = 20.;
    rainDesc.beginColor = glm::vec4(0, 0, 1, 1);
    rainDesc.endColor = glm::vec4(0, 0, 1, 1);
    rainDesc.minLifeTime = 0.5f;
    rainDesc.maxLifeTime = 1.f;

    rain = ParticleManager::Instance()->CreateEmitter({0, 15.f, 0}, rainDesc, 0.1f);

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
        ParticleDesc fireDesc;
        fireDesc.amount = 100;
        fireDesc.spawnRadius = 0.9f;
        fireDesc.particleBeginSize = 0.15f;
        fireDesc.particleEndSize = 0.f;

        fireDesc.rotationSpeed = 10.f;
        fireDesc.floorDir = glm::vec3(-0.25f, 0.5f, -0.25f);
        fireDesc.ceilDir = glm::vec3(0.25f, 1, 0.25f);
        fireDesc.minSpeed = 0.33f;
        fireDesc.maxSpeed = 3.f;
        fireDesc.beginColor = glm::vec4(1, 0, 0, 1);
        fireDesc.endColor = glm::vec4(1, 1, 0, 1);
        fireDesc.minLifeTime = 1.f;
        fireDesc.maxLifeTime = 3.f;

        ParticleManager::Instance()->Spawn({ 0, 0, 0 }, fireDesc);
    }
}

void SceneParticlesTest::OnUpdate(float dt)
{
    obj->transform.AddYPR(glm::vec3(0, 0.5f*dt, 0));
    obj->transform.Move(obj->transform.GetForward() * 1.5f * dt);
    fire.center = obj->transform.GetPosition();

    ParticleManager::Instance()->UpdateEmitter(fire, dt);
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
    world.SetIdentity();
    _pDevice->RegisterCommand(mesh_particle, world.GetMatrix(), m_unlitParticle.get());
    _pDevice->EndPass();
}
