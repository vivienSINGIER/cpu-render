#include "TexturedBP_Scene.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

void TexturedBP_Scene::InitLightScene()
{
    //m_lightData.lights[0] = MakeDirectionalLight({1.0f, -1.0f, 1.0f}, 10.0f, Color::WHITE);
    //m_lightData.lights[1] = MakePointLight({5.0f, 0.0f, 5.0f}, 0.5f, { 5.0f, 25.0f }, Color::RED);
    m_lightData.ambient = 10.0f;
}

void TexturedBP_Scene::OnInit(Device* _pDevice)
{
    InitLightScene();
    
    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_presentTarget = m_mainPass.fb.gBuffer[0];
    
    m_skyboxPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_skyboxPass.state.depthTest = false;
    m_skyboxPass.state.depthWrite = false;


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

    //Handle diff = TextureFactory::Instance()->CreateTexture("dark_rock_diffuse", "Assets/Textures/Gold/diffuse.png", true);
    //Handle ambient = TextureFactory::Instance()->CreateTexture("dark_rock_ambient", "Assets/Textures/Gold/ambient.png");
    //Handle normal = TextureFactory::Instance()->CreateTexture("dark_rock_normal", "Assets/Textures/Gold/normal.png");
    ////Handle roughness = TextureFactory::Instance()->CreateTexture("dark_rock_roughness", "Assets/Textures/Gold/roughness.png");
    //Handle displacement = TextureFactory::Instance()->CreateTexture("dark_rock_displacement", "Assets/Textures/Gold/displacement.png");
    ////Handle metalness = TextureFactory::Instance()->CreateTexture("dark_rock_metalness", "Assets/Textures/Gold/metalness.png");
    
    LitT_BP_Cube_PS::MaterialCB cb = {
        {1.0f, 1.0f, 1.0f, 1.0f}, 
            Lighting::g_fresnelR0Templates[PLASTIC], 
                0.9f, 0.02f, 0.9f
    };

    //Cubemap cubemap = Cubemap("Skybox", "Assets/Textures/Cubemap/Front.png", "Assets/Textures/Cubemap/Back.png",
    //    "Assets/Textures/Cubemap/Left.png", "Assets/Textures/Cubemap/Right.png",
    //    "Assets/Textures/Cubemap/Up.png", "Assets/Textures/Cubemap/Down.png");

    Cubemap cubemap = Cubemap("Skybox", "Assets/Textures/Cubemap/FishPond/posz.jpg", "Assets/Textures/Cubemap/FishPond/negz.jpg",
        "Assets/Textures/Cubemap/FishPond/negx.jpg", "Assets/Textures/Cubemap/FishPond/posx.jpg",
        "Assets/Textures/Cubemap/FishPond/posy.jpg", "Assets/Textures/Cubemap/FishPond/negy.jpg", true);

    Mesh m = GeometryFactory::Instance()->BuildUVSphere(10, 10, true);
    Mesh cube = GeometryFactory::Instance()->BuildCube();

    m_mat = ShaderFactory::Instance()->CreateMaterial("LitT_BP_Cube");
    m_mat->SetParams<LitT_BP_Cube_PS::MaterialCB>(LitT_BP_Cube_PS::CB_SLOT_mat, cb);
    m_mat->SetTexture(LitT_BP_Cube_PS::TEX_SLOT_diffuse, diff);
    m_mat->SetTexture(LitT_BP_Cube_PS::TEX_SLOT_ambient, ambient);
    m_mat->SetTexture(LitT_BP_Cube_PS::TEX_SLOT_normal, normal);
    
    //m_mat->SetTexture(LitT_BP_Cube_PS::TEX_SLOT_roughness, roughness);
    m_mat->SetTexture(LitT_BP_Cube_PS::TEX_SLOT_displacement, displacement);
    //m_mat->SetTexture(LitT_BP_Cube_PS::TEX_SLOT_metal, metalness);
    m_mat->SetTexture(LitT_BP_Cube_PS::TEX_SLOT_shadow, m_shadowPass.fb.depthBuffer);
    m_mat->SetTexture(LitT_BP_Cube_PS::TEX_SLOT_cubemap, cubemap.GetTexture());
	
    Object* o = CreateObject(DYNAMIC);
    o->mesh = m;
    o->mat = m_mat.get();
    o->transform.SetPosition(glm::vec3(0, 0, 0));

    Object* floor = CreateObject(DYNAMIC);
    floor->mesh = cube;
    floor->mat = m_mat.get();
    floor->transform.SetPosition(glm::vec3(0.0f, -5.0f, 0.f));
    floor->transform.SetScale(glm::vec3(25.0f, 0.2f, 25.0f));

    SetSkybox(cubemap);
}

void TexturedBP_Scene::OnHandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_P))
    {
        togglePostProc = !togglePostProc;
    }
}

void TexturedBP_Scene::OnUpdate(float dt)
{
}

void TexturedBP_Scene::OnDraw(Device* _pDevice)
{
    Frustum shadowFrustum;
    glm::mat4x4 lightSpace = Lighting::ComputeDirLightViewProj(m_shadowPass.passData.view, m_camera.GetShadowProj(),
                                        m_lightData.lights[0].light.direction, m_shadowPass.fb.width, shadowFrustum, 0.0f);

    m_camera.FillData(&m_mainPass.passData);
    m_mainPass.passData.deltaTime = m_deltaTime;
    m_mainPass.passData.totalTime = m_totalTime;
    m_mainPass.passData.lightSpace = lightSpace;

    m_camera.FillData(&m_shadowPass.passData);
    m_shadowPass.passData.viewProj = lightSpace;

    if (toggleSkybox)
    {
        m_camera.FillData(&m_skyboxPass.passData);
        _pDevice->BeginPass(m_skyboxPass);
            _pDevice->RegisterCommand(skyboxObject->mesh, skyboxObject->transform.GetMatrix(), skyboxObject->mat);
        _pDevice->EndPass();
    }

    _pDevice->BeginPass(m_shadowPass);
        DrawObjects(_pDevice, shadowFrustum);
    _pDevice->EndPass();

    _pDevice->BeginPass(m_mainPass);
        DrawObjects(_pDevice, m_camera.GetFrustum());
    _pDevice->EndPass();

    if (togglePostProc)
    {
        _pDevice->BeginPass(m_postProcPass);
        _pDevice->EndPass();
    }
}

void TexturedBP_Scene::SetSkybox(Cubemap _cubemap)
{
    skyboxObject = new Object();
    m_skyboxMat = ShaderFactory::Instance()->CreateMaterial("UnlitSkybox");
    m_skyboxMat->SetTexture(UnlitSkybox_PS::TEX_SLOT_albedo, _cubemap.GetTexture());

    skyboxObject->mat = m_skyboxMat.get();
    skyboxObject->mesh = GeometryFactory::Instance()->BuildSkybox();
    skyboxObject->transform.SetScale(50.0f);
    toggleSkybox = true;
    m_mainPass.clearFlags.clearGBuffer[0] = false;
}

