#include "PBR_Scene.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

void PBR_Scene::InitLightScene()
{
    m_lightData.lights[0] = MakePointLight({3.0f, -3.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    m_lightData.lights[1] = MakePointLight({3.0f, 3.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    m_lightData.lights[2] = MakePointLight({-3.0f, 3.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    m_lightData.lights[3] = MakePointLight({-3.0f, -3.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    m_lightData.ambient = 0.03f;
}

void PBR_Scene::OnInit(Device* _pDevice)
{
    InitLightScene();
    
    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_presentTarget = m_mainPass.fb.gBuffer[0];
    
    m_shadowPass.fb = _pDevice->CreateFrameBuffer(1024, 1024, 0, true);
    m_shadowPass.state.cullMode = CullMode::FRONT;
    
    m_postProcPass.isCompute = true;
    
    _pDevice->SetLightData(m_lightData);
    
    m_camera.SetShadowDistance(25.0f);
    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));

    Handle diff = TextureFactory::Instance()->CreateTexture("dark_rock_diffuse", "Assets/Textures/DarkRock/diffuse.png", true);
    Handle ambient = TextureFactory::Instance()->CreateTexture("dark_rock_ambient", "Assets/Textures/DarkRock/ambient.png");
    Handle normal = TextureFactory::Instance()->CreateTexture("dark_rock_normal", "Assets/Textures/DarkRock/normal.png");
    Handle roughness = TextureFactory::Instance()->CreateTexture("dark_rock_roughness", "Assets/Textures/DarkRock/roughness.png");
    Handle displacement = TextureFactory::Instance()->CreateTexture("dark_rock_displacement", "Assets/Textures/DarkRock/displacement.png");
    Handle metalness = TextureFactory::Instance()->CreateTexture("dark_rock_metalness", "Assets/Textures/DarkRock/metalness.png");
    
    Mesh m = GeometryFactory::Instance()->BuildUVSphere(32, 32, true);

    m_mat = ShaderFactory::Instance()->CreateMaterial("PBR");
    UpdateMaterial();

    Object* o = CreateObject(DYNAMIC);
    o->mat = m_mat.get();
    o->mesh = m;
    o->transform.SetPosition(glm::vec3(0.0f));
    o->transform.SetScale(3.0f);
}

void PBR_Scene::UpdateMaterial()
{
    // roughness 0 makes the GGX lobe a dirac, so the highlight vanishes
    m_roughness = glm::clamp(m_roughness, 0.05f, 1.0f);
    m_metalness = glm::clamp(m_metalness, 0.0f, 1.0f);

    PBR_PS::MaterialCB cb = { {0.5f, 0.0f, 0.0f}, m_metalness, m_roughness, 0.2f };
    m_mat->SetParams<PBR_PS::MaterialCB>(PBR_PS::CB_SLOT_mat, cb);

    std::cout << "Roughness : " << m_roughness << " | Metalness : " << m_metalness << std::endl;
}

void PBR_Scene::OnHandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
        return;

    if (event.key.key == SDLK_P)
    {
        togglePostProc = !togglePostProc;
    }

    const float step = 0.05f;

    if (event.key.key == SDLK_1) { m_roughness -= step; UpdateMaterial(); }
    if (event.key.key == SDLK_2) { m_roughness += step; UpdateMaterial(); }
    if (event.key.key == SDLK_3) { m_metalness -= step; UpdateMaterial(); }
    if (event.key.key == SDLK_4) { m_metalness += step; UpdateMaterial(); }
}

void PBR_Scene::OnUpdate(float dt)
{
    
}

void PBR_Scene::OnDraw(Device* _pDevice)
{
    Frustum shadowFrustum;
    glm::mat4x4 lightSpace = Lighting::ComputeDirLightViewProj(m_shadowPass.passData.view, m_camera.GetShadowProj(),
                                        m_lightData.lights[0].light.direction, m_shadowPass.fb.width, shadowFrustum, 0.0f);

    m_camera.FillData(&m_mainPass.passData);
    m_mainPass.passData.deltaTime = m_deltaTime;
    m_mainPass.passData.totalTime = m_totalTime;
    m_mainPass.passData.lightSpace = lightSpace;

    _pDevice->BeginPass(m_mainPass);
        DrawObjects(_pDevice, m_camera.GetFrustum());
    _pDevice->EndPass();

    if (togglePostProc)
    {
        _pDevice->BeginPass(m_postProcPass);
        _pDevice->EndPass();
    }
}

