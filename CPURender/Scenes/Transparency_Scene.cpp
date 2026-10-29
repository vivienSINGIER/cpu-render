#include "Transparency_Scene.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

static const char* g_blendModeNames[Blending::COUNT] = {
    "OPAQUE", "ALPHA", "PREMULTALPHA", "ADDITIVE", "ADDITIVE_ALPHA", "MULTIPLICATIVE",
    "MULTIPLICATIVE_2X", "SCREEN", "SUBTRACTIVE", "MIN", "MAX"
};

void Transparency_Scene::InitLightScene()
{
    m_lightData.lights[0] = MakeDirectionalLight({1.0f, -1.0f, 1.0f}, 2.0f, Color::WHITE);
    m_lightData.ambient = 0.0f;
}

void Transparency_Scene::OnInit(Device* _pDevice)
{
    InitLightScene();

    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_presentTarget = m_mainPass.fb.gBuffer[0];
    m_mainPass.state.blendMode = (Blending::BlendMode)m_blendIndex;
    m_mainPass.state.cullMode = CullMode::BACK;

    _pDevice->SetLightData(m_lightData);

    Cubemap cubemap = Cubemap("Skybox", "Assets/Textures/Cubemap/FishPond/posz.jpg", "Assets/Textures/Cubemap/FishPond/negz.jpg",
        "Assets/Textures/Cubemap/FishPond/negx.jpg", "Assets/Textures/Cubemap/FishPond/posx.jpg",
        "Assets/Textures/Cubemap/FishPond/posy.jpg", "Assets/Textures/Cubemap/FishPond/negy.jpg");

    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));
    
    LitC_BP_Cube_PS::MaterialCB floorCB  = { {0.6f, 0.6f, 0.6f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.8f, 0.0f, 0.0f, 1.7f, false };
    LitC_BP_Cube_PS::MaterialCB opaqueCB = { {0.9f, 0.2f, 0.2f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.5f, 0.0f, 0.0f, 1.7f, false };
    LitC_BP_Cube_PS::MaterialCB glassCB  = { {0.2f, 0.4f, 0.9f, 1.0f}, Lighting::g_fresnelR0Templates[GLASS],   0.1f, 0.0f, 1.0f, 1.7f, true };
    LitC_BP_Cube_PS::MaterialCB tintedCB = { {1.0f, 1.0f, 1.0f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.4f, 0.0f, 0.95f, 1.7f, true };
    LitC_BP_Cube_PS::MaterialCB metalCB  = { {0.9f, 0.8f, 0.3f, 1.0f}, Lighting::g_fresnelR0Templates[CRYSTAL], 0.2f, 0.5f, 0.5f, 1.7f, true };

    Mesh sphere = GeometryFactory::Instance()->BuildUVSphere(20, 20);
    Mesh cube = GeometryFactory::Instance()->BuildCube();
    Mesh donut = GeometryFactory::Instance()->BuildDonut(1.0f, 0.35f, 24, 12);

    m_floorMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Cube");
    m_floorMat->SetParams<LitC_BP_Cube_PS::MaterialCB>(LitC_BP_Cube_PS::CB_SLOT_mat, floorCB);
    m_floorMat->SetTexture(LitC_BP_Cube_PS::TEX_SLOT_cubemap, cubemap.GetTexture());

    m_opaqueMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Cube");
    m_opaqueMat->SetParams<LitC_BP_Cube_PS::MaterialCB>(LitC_BP_Cube_PS::CB_SLOT_mat, opaqueCB);
    m_opaqueMat->SetTexture(LitC_BP_Cube_PS::TEX_SLOT_cubemap, cubemap.GetTexture());

    m_glassMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Cube");
    m_glassMat->SetParams<LitC_BP_Cube_PS::MaterialCB>(LitC_BP_Cube_PS::CB_SLOT_mat, glassCB);
    m_glassMat->SetTexture(LitC_BP_Cube_PS::TEX_SLOT_cubemap, cubemap.GetTexture());

    m_tintedMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Cube");
    m_tintedMat->SetParams<LitC_BP_Cube_PS::MaterialCB>(LitC_BP_Cube_PS::CB_SLOT_mat, tintedCB);
    m_tintedMat->SetTexture(LitC_BP_Cube_PS::TEX_SLOT_cubemap, cubemap.GetTexture());

    m_metalMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Cube");
    m_metalMat->SetParams<LitC_BP_Cube_PS::MaterialCB>(LitC_BP_Cube_PS::CB_SLOT_mat, metalCB);
    m_metalMat->SetTexture(LitC_BP_Cube_PS::TEX_SLOT_cubemap, cubemap.GetTexture());

    Object* floor = CreateObject(DYNAMIC);
    floor->mesh = cube;
    floor->mat = m_floorMat.get();
    floor->transform.SetPosition(glm::vec3(0.0f, -2.0f, 0.f));
    floor->transform.SetScale(glm::vec3(25.0f, 0.2f, 25.0f));

    Object* back = CreateObject(DYNAMIC);
    back->mesh = sphere;
    back->mat = m_opaqueMat.get();
    back->transform.SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
    back->transform.SetScale(glm::vec3(1.5f));

    Object* metal = CreateObject(DYNAMIC);
    metal->mesh = donut;
    metal->mat = m_metalMat.get();
    metal->transform.SetPosition(glm::vec3(2.5f, 0.0f, 1.0f));

    Object* tinted = CreateObject(DYNAMIC);
    tinted->mesh = cube;
    tinted->mat = m_tintedMat.get();
    tinted->transform.SetPosition(glm::vec3(-2.0f, 0.0f, 0.0f));

    Object* glass = CreateObject(DYNAMIC);
    glass->mesh = sphere;
    glass->mat = m_glassMat.get();
    glass->transform.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

    m_skyboxPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_skyboxPass.state.depthTest = false;
    m_skyboxPass.state.depthWrite = false;
    //m_skyboxPass.clearFlags.clearGBuffer[0] = false;
    m_skyboxPass.clearFlags.clearDepth = false;

    skyboxObject = new Object();
    m_skyboxMat = ShaderFactory::Instance()->CreateMaterial("UnlitSkybox");
    m_skyboxMat->SetTexture(UnlitSkybox_PS::TEX_SLOT_albedo, cubemap.GetTexture());

    skyboxObject->mat = m_skyboxMat.get();
    skyboxObject->mesh = GeometryFactory::Instance()->BuildSkybox();
    skyboxObject->transform.SetScale(50.0f);
    m_toggleSkybox = true;
    m_mainPass.clearFlags.clearGBuffer[0] = false;
}

void Transparency_Scene::CycleBlendMode(int _step)
{
    m_blendIndex = (m_blendIndex + _step + Blending::COUNT) % Blending::COUNT;
    m_mainPass.state.blendMode = (Blending::BlendMode)m_blendIndex;

    std::cout << "Blend mode : " << g_blendModeNames[m_blendIndex] << std::endl;
}

void Transparency_Scene::OnHandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
        return;

    if (event.key.key == SDLK_A) CycleBlendMode(-1);
    if (event.key.key == SDLK_E) CycleBlendMode(1);
}

void Transparency_Scene::OnUpdate(float dt)
{
}

void Transparency_Scene::OnDraw(Device* _pDevice)
{
    m_camera.FillData(&m_mainPass.passData);
    m_mainPass.passData.deltaTime = m_deltaTime;
    m_mainPass.passData.totalTime = m_totalTime;

    if (m_toggleSkybox)
    {
        m_camera.FillData(&m_skyboxPass.passData);
        _pDevice->BeginPass(m_skyboxPass);
        _pDevice->RegisterCommand(skyboxObject->mesh, skyboxObject->transform.GetMatrix(), skyboxObject->mat);
        _pDevice->EndPass();
    }

    _pDevice->BeginPass(m_mainPass);
        DrawObjects(_pDevice, m_camera.GetFrustum());
    _pDevice->EndPass();
}
