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
    m_lightData.lights[0] = MakeDirectionalLight({1.0f, -1.0f, 1.0f}, 0.75f, Color::WHITE);
    m_lightData.ambient = 0.2f;
}

void Transparency_Scene::OnInit(Device* _pDevice)
{
    InitLightScene();

    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_presentTarget = m_mainPass.fb.gBuffer[0];
    m_mainPass.state.blendMode = (Blending::BlendMode)m_blendIndex;
    m_mainPass.state.cullMode = CullMode::BACK;

    _pDevice->SetLightData(m_lightData);

    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));
    
    LitC_BP_PS::MaterialCB floorCB  = { {0.6f, 0.6f, 0.6f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.8f, 0.0f, 0.0f, false };
    LitC_BP_PS::MaterialCB opaqueCB = { {0.9f, 0.2f, 0.2f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.5f, 0.0f, 0.0f, false };
    LitC_BP_PS::MaterialCB glassCB  = { {0.2f, 0.4f, 0.9f, 1.0f}, Lighting::g_fresnelR0Templates[GLASS],   0.1f, 0.0f, 0.6f, true };
    LitC_BP_PS::MaterialCB tintedCB = { {0.2f, 0.9f, 0.3f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.4f, 0.0f, 0.4f, true };
    LitC_BP_PS::MaterialCB metalCB  = { {0.9f, 0.8f, 0.3f, 1.0f}, Lighting::g_fresnelR0Templates[CRYSTAL], 0.2f, 0.5f, 0.5f, true };

    Mesh sphere = GeometryFactory::Instance()->BuildUVSphere(20, 20);
    Mesh cube = GeometryFactory::Instance()->BuildCube();
    Mesh donut = GeometryFactory::Instance()->BuildDonut(1.0f, 0.35f, 24, 12);

    m_floorMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP");
    m_floorMat->SetParams<LitC_BP_PS::MaterialCB>(LitC_BP_PS::CB_SLOT_mat, floorCB);

    m_opaqueMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP");
    m_opaqueMat->SetParams<LitC_BP_PS::MaterialCB>(LitC_BP_PS::CB_SLOT_mat, opaqueCB);

    m_glassMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP");
    m_glassMat->SetParams<LitC_BP_PS::MaterialCB>(LitC_BP_PS::CB_SLOT_mat, glassCB);

    m_tintedMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP");
    m_tintedMat->SetParams<LitC_BP_PS::MaterialCB>(LitC_BP_PS::CB_SLOT_mat, tintedCB);

    m_metalMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP");
    m_metalMat->SetParams<LitC_BP_PS::MaterialCB>(LitC_BP_PS::CB_SLOT_mat, metalCB);
    
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
    m_camera.FillData(&m_mainPass.view);
    m_mainPass.view.deltaTime = m_deltaTime;
    m_mainPass.view.totalTime = m_totalTime;

    _pDevice->BeginPass(m_mainPass);
        DrawObjects(_pDevice, m_camera.GetFrustum());
    _pDevice->EndPass();
}
