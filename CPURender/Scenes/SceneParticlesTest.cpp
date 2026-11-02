#include "SceneParticlesTest.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

#include "../Shaders/ComputeShaders/ParticleCS.hpp"
#include "../Factories/BufferFactory.h"

#include "../Animation/AnimationData.h"

void SceneParticlesTest::OnInit(Device* _pDevice)
{
#pragma region PARTICLE_INIT
    m_lightData.lights[0] = MakeDirectionalLight({ 1.0f, -1.0f, 1.0f }, 1.f, Color::WHITE);
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
    // m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_ambient, ambient);
    // m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_normal, normal);

    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_roughness, roughness);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_displacement, displacement);

    Mesh cube = GeometryFactory::Instance()->BuildCube();
    Object* floor = CreateObject(DYNAMIC);
    floor->mesh = cube;
    floor->mat = m_mat.get();
    floor->transform.SetPosition(glm::vec3(0.0f, -2.f, 0.f));
    floor->transform.SetScale(glm::vec3(25.0f, 0.2f, 25.0f));

    ParticleDesc fireDesc;
    fireDesc.amount = 5;
    fireDesc.spawnRadius = 0.75f;
    fireDesc.particleBeginSize = 0.1f;
    fireDesc.particleEndSize = 0.01f;

    fireDesc.rotationSpeed = 10.f;
    fireDesc.floorDir = glm::vec3(-0.25f, 0.5f, -0.25f);
    fireDesc.ceilDir = glm::vec3(0.25f, 1, 0.25f);
    fireDesc.minSpeed = 0.33f;
    fireDesc.maxSpeed = 4.f;
    fireDesc.beginColor = glm::vec4(1, 0, 0, 1.f);
    fireDesc.endColor = glm::vec4(1, 1, 0, 0.f);
    fireDesc.minLifeTime = 1.f;
    fireDesc.maxLifeTime = 3.f;

    fire = ParticleManager::Instance()->CreateEmitter({ 0, 2, 0 }, fireDesc, 0.0f);

    ParticleDesc rainDesc;
    rainDesc.amount = 200;
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
#pragma endregion PARTICLE_INIT

    Model test("Assets/Fbx/dance.fbx");
    std::cout << "Bones = " << test.GetBoneCount() << std::endl;

    ShaderFactory::Instance()->RegisterGroup<LitT_BP_VS, LitT_BP_PS>("TestMaterial");

    model_mat = ShaderFactory::Instance()->CreateMaterial(std::string("TestMaterial"));
    model_mat->SetParams<LitT_BP_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);

    Handle tex = TextureFactory::Instance()->CreateTexture("fbx_tex", "Assets/Fbx/Character_Male_34.png");
    model_mat->SetTexture(LitT_BP_PS::TEX_SLOT_diffuse, tex);

    obj = CreateObject(DYNAMIC);
    obj->mesh = test.meshes[0];
    obj->mat = model_mat.get();

    obj->transform.SetIdentity();
    obj->transform.SetScale(2.f);
  //  obj->transform.SetScale(0.025f);
    obj->transform.SetYPR(glm::vec3(-pi<float>()*0.5f, pi<float>(), 0));
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
    obj->transform.AddYPR(glm::vec3(0, 0, 1.f*dt));
    //obj->transform.Move(obj->transform.GetForward() * 1.5f * dt);
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
    ParticleManager::Instance()->Draw(_pDevice);
    _pDevice->EndPass();
}
