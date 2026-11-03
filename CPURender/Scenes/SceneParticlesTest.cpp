#include "SceneParticlesTest.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

#include "../Shaders/ComputeShaders/ParticleCS.hpp"
#include "../Factories/BufferFactory.h"

#include "../Datas/SkinnedObject.h"

void SceneParticlesTest::OnInit(Device* _pDevice)
{
#pragma region PARTICLE_INIT
    m_lightData.lights[0] = MakeDirectionalLight({ 1.0f, -1.0f, 1.0f }, 10.f, Color::WHITE);
    m_lightData.ambient = 0.2f;

    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_presentTarget = m_mainPass.fb.gBuffer[0];

    m_particlesPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_particlesPass.isCompute = true;
    m_particlesPass.state.blendMode = Blending::ALPHA;

    _pDevice->SetLightData(m_lightData);

    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));

    Handle diff = TextureFactory::Instance()->CreateTexture("dark_rock_diffuse", "Assets/Textures/DarkRock/diffuse.png", true);
    Handle ambient = TextureFactory::Instance()->CreateTexture("dark_rock_ambient", "Assets/Textures/DarkRock/ambient.png");
    Handle normal = TextureFactory::Instance()->CreateTexture("dark_rock_normal", "Assets/Textures/DarkRock/normal.png");
    Handle roughness = TextureFactory::Instance()->CreateTexture("dark_rock_roughness", "Assets/Textures/DarkRock/roughness.png");
    Handle displacement = TextureFactory::Instance()->CreateTexture("dark_rock_displacement", "Assets/Textures/DarkRock/displacement.png");

    LitT_BP_PS::MaterialCB cb = {
        {1.0f, 1.0f, 1.0f, 1.0f},
            Lighting::g_fresnelR0Templates[PLASTIC],
                0.5f, 0.02f, 0.8f
    };

    UnlitC_PS::UnlitColor unlit_color = { glm::vec4(0, 1, 1, 1) };

    m_unlit = ShaderFactory::Instance()->CreateMaterial("UnlitC");
    m_unlit->SetParams<UnlitC_PS::UnlitColor>(UnlitC_PS::CB_SLOT_mat, unlit_color);

    m_mat = ShaderFactory::Instance()->CreateMaterial("LitT_BP");
    m_mat->SetParams<LitT_BP_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_diffuse, diff);
    //m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_ambient, ambient);
    //m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_normal, normal);

    //m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_roughness, roughness);
    //m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_displacement, displacement);

    Mesh cube = GeometryFactory::Instance()->BuildCube();
    Object* floor = CreateObject(DYNAMIC);
    floor->mesh = cube;
    floor->mat = m_mat.get();
    floor->transform.SetPosition(glm::vec3(0.0f, -2.f, 0.f));
    floor->transform.SetScale(glm::vec3(25.0f, 0.2f, 25.0f));

    ParticleDesc heartDesc;
    {
    heartDesc.amount = 1;
    heartDesc.spawnRadius = 0.75f;
    heartDesc.particleBeginSize = 0.02f;
    heartDesc.particleEndSize = 0.5f;

    heartDesc.rotationSpeed = 0.f;
    heartDesc.floorDir = glm::vec3(-0.5f, 1.f, -0.5f);
    heartDesc.ceilDir = glm::vec3(0.5f, 1, 0.5f);
    heartDesc.minSpeed = 1.f;
    heartDesc.maxSpeed = 3.f;
    heartDesc.beginColor = glm::vec4(1, 0, 0, 1);
    heartDesc.endColor = glm::vec4(1, 1, 0, 1);
    heartDesc.minLifeTime = 2.f;
    heartDesc.maxLifeTime = 3.f;
    }
    std::string heart = ParticleManager::Instance()->BindSlot("heartUwU", "Assets/Textures/Particles/heart.png");
    fire = ParticleManager::Instance()->CreateEmitter({ 0, 2.5f, 0 }, heartDesc, 0.0f, heart);

    ///

    ParticleDesc test;
    {
        test.amount = 5;
        test.spawnRadius = 0.75f;
        test.particleBeginSize = 0.1f;
        test.particleEndSize = 0.1f;

        test.rotationSpeed = 10.f;
        test.floorDir = glm::vec3(-1, -1, -1);
        test.ceilDir = glm::vec3(1, 1, 1);
        test.minSpeed = 0.3f;
        test.maxSpeed = 3.f;
        test.beginColor = glm::vec4(1, 0, 1, 1);
        test.endColor = glm::vec4(0, 0, 1, 1);
        test.minLifeTime = 1.f;
        test.maxLifeTime = 2.f;
    }
    ParticleEmitter* d = ParticleManager::Instance()->CreateEmitter({ 0.f, 0.f, 0 }, test, 0.0f);

    ///

    ParticleDesc r;
    {
        r.amount = 50;
        r.spawnRadius = 20.f;
        r.particleBeginSize = 0.2f;
        r.particleEndSize = 0.01f;

        r.rotationSpeed = 10.f;
        r.floorDir = glm::vec3(-0.25f, -1, -0.25f);
        r.ceilDir = glm::vec3(0.25f, -1, 0.25f);
        r.minSpeed = 15.f;
        r.maxSpeed = 20.f;
        r.beginColor = glm::vec4(1, 1, 1, 1);
        r.endColor = glm::vec4(1, 1, 1, 1);
        r.minLifeTime = 1.f;
        r.maxLifeTime = 2.f;

    }
    std::string rainbow = ParticleManager::Instance()->BindSlot("rainbow", "Assets/Textures/Particles/rainbow.png");
    ParticleEmitter* e = ParticleManager::Instance()->CreateEmitter({ 0.f, 15.f, 0 }, r, 0.0f, rainbow);

#pragma endregion PARTICLE_INIT

    ShaderFactory::Instance()->RegisterGroup<LitT_BP_VS, LitT_BP_PS>("TestMaterial");
    model_mat = ShaderFactory::Instance()->CreateMaterial(std::string("TestMaterial"));
    model_mat->SetParams<LitT_BP_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);

    Handle tex = TextureFactory::Instance()->CreateTexture("Character_Male_34", "Assets/Fbx/Character_Male_34.png");
    model_mat->SetTexture(LitT_BP_PS::TEX_SLOT_diffuse, tex);

    obj = CreateSkinnedObject("Assets/Fbx/dance.fbx");
    obj->AddAnimation("Dancing", "Assets/Fbx/dance.fbx");
    obj->animator.PlayAnimation(obj->GetAnimation("Dancing"));

    obj->mat = model_mat.get();
    obj->transform.SetPosition(glm::vec3(0, -1.9f, 0));
    obj->transform.SetYPR(glm::vec3(0, pi<float>(), 0));
    obj->transform.SetScale(2.f);

    SkinnedObject* t = CreateSkinnedObject("Assets/Fbx/theHand.fbx");
    t->AddAnimation("Hold", "Assets/Fbx/theHand.fbx");
    t->animator.PlayAnimation(t->GetAnimation("Hold"));

    hand_mat = ShaderFactory::Instance()->CreateMaterial(std::string("TestMaterial"));
    hand_mat->SetParams<LitT_BP_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);

    t->mat = hand_mat.get();
    t->transform.SetPosition(glm::vec3(-1.5f, 1.f, 0));
    t->transform.SetScale(0.02f);
}

void SceneParticlesTest::OnHandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_A))
    {
    }
}

void SceneParticlesTest::OnUpdate(float dt)
{
    obj->transform.AddYPR(glm::vec3(0, dt, 0));
    obj->transform.Move(obj->transform.GetForward(), dt*3.f);
    fire->center = obj->transform.GetPosition();

    ParticleManager::Instance()->Update(dt);
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
    ParticleManager::Instance()->Draw(_pDevice);
    _pDevice->EndPass();
}
