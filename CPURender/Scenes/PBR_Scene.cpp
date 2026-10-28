#include "PBR_Scene.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

void PBR_Scene::InitLightScene()
{
    m_lightData.lights[0] = MakePointLight({3.0f, -2.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    m_lightData.lights[1] = MakePointLight({3.0f, -2.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    m_lightData.lights[1] = MakePointLight({-2.0f, -2.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    m_lightData.lights[1] = MakePointLight({-2.0f, 3.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
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
    
    Mesh m = GeometryFactory::Instance()->BuildUVSphere(10, 10, true);
    Mesh cube = GeometryFactory::Instance()->BuildCube();
    
    for (int i = -2; i < 3; i++)
    {
        for (int j = -2; j < 3; j++)
        {
            int index = i + 2 + 5 * (j + 2);
            
            float x = float(i + 2) / 5.0f;
            float y = float(j + 2) / 5.0f;
            
            PBR_PS::MaterialCB cb = {
                {1.0f, 0.0f, 0.0f}, 
                1.0f * (1.0f - x), 1.0f * (1.0f - y), 0.2f
            };
    
            m_mats[index] = ShaderFactory::Instance()->CreateMaterial("PBR");
            m_mats[index]->SetParams<PBR_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);
            
            Object* o = CreateObject(DYNAMIC);
            o->mat = m_mats[index].get();
            o->mesh = m;
            o->transform.SetPosition(glm::vec3(i * 2, j * 2, 0.0f));
        }
    }
    
}

void PBR_Scene::OnHandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_P))
    {
        togglePostProc = !togglePostProc;
    }
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

