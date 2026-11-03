#include "Transparency_Scene.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

enum DrawMask {
    OPAQUE = 0b00000001,
    TRANSPARENT = 0b00000010,
};

static const char* g_blendModeNames[Blending::COUNT] = {
    "OPAQUE", "ALPHA", "PREMULTALPHA", "ADDITIVE", "ADDITIVE_ALPHA", "MULTIPLICATIVE",
    "MULTIPLICATIVE_2X", "SCREEN", "SUBTRACTIVE", "MIN", "MAX"
};

void Transparency_Scene::InitLightScene()
{
    m_lightData.lights[0] = MakeDirectionalLight({1.0f, -1.0f, 1.0f}, 2.0f, Color::WHITE);
    m_lightData.ambient = 0.03f;
}

void Transparency_Scene::InitPassDesc(Device* _pDevice)
{
    FrameBuffer refractedFB = _pDevice->CreateFrameBuffer(WIDTH, HEIGHT, 1, true, glm::vec4(0, 0, 0, 1));
    m_skyboxPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_skyboxPass.fb.depthBuffer = _pDevice->GetDefaultFrameBuffer().depthBuffer;
    m_skyboxPass.state.depthTest = true;
    m_skyboxPass.state.depthWrite = false;
    m_skyboxPass.clearFlags.clearGBuffer[0] = false;
    m_skyboxPass.clearFlags.clearDepth = false;

    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_mainPass.fb.depthBuffer = _pDevice->GetDefaultFrameBuffer().depthBuffer;
    m_mainPass.state.blendMode = Blending::BlendMode::OPAQUE;
    m_mainPass.state.cullMode = CullMode::BACK;

    m_refractionPass.fb = refractedFB;
    m_refractionPass.state.blendMode = Blending::BlendMode::OPAQUE;
    m_refractionPass.state.cullMode = CullMode::BACK;
    m_refractionPass.clearFlags.clearDepth = false;

    m_postProcPass.isCompute = true;
    m_depthCopyPass.isCompute = true;
}

void Transparency_Scene::InitMaterials()
{
    LitC_BP_Cube_PS::MaterialCB floorCB = { {0.6f, 0.6f, 0.6f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.8f, 0.0f, 0.0f, 1.7f, false };
    LitC_BP_Cube_PS::MaterialCB opaqueCB = { {1.0f, 0.2f, 0.2f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.5f, 0.0f, 0.0f, 1.7f, false };
    LitC_BP_Transparent_PS::MaterialCB transparentCB = { {1.0f, 1.0f, 1.0f, 1.0f}, Lighting::g_fresnelR0Templates[CRYSTAL], 0.2f, 0.5f, 0.8f, 1.02f, true, {WIDTH, HEIGHT} };

    m_postProcMat = ShaderFactory::Instance()->CreateComputeMaterial("FBFusion");
    m_postProcMat->SetRWTexture(FB_Fusion_CS::UAV_SLOT_target1, m_mainPass.fb.gBuffer[0]);
    m_postProcMat->SetRWTexture(FB_Fusion_CS::UAV_SLOT_target2, m_refractionPass.fb.gBuffer[0]);
    m_postProcMat->SetRWTexture(FB_Fusion_CS::UAV_SLOT_target1db, m_mainPass.fb.depthBuffer);
    m_postProcMat->SetRWTexture(FB_Fusion_CS::UAV_SLOT_target2db, m_refractionPass.fb.depthBuffer);

    m_depthCopyMat = ShaderFactory::Instance()->CreateComputeMaterial("TexCopy");
    m_depthCopyMat->SetRWTexture(TexCopy_CS::UAV_SLOT_source, m_mainPass.fb.depthBuffer);
    m_depthCopyMat->SetRWTexture(TexCopy_CS::UAV_SLOT_target, m_refractionPass.fb.depthBuffer);

    m_floorMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Cube");
    m_floorMat->SetParams<LitC_BP_Cube_PS::MaterialCB>(LitC_BP_Cube_PS::CB_SLOT_mat, floorCB);
    m_floorMat->SetTexture(LitC_BP_Cube_PS::TEX_SLOT_cubemap, m_cubemap.GetTexture());

    m_opaqueMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Cube");
    m_opaqueMat->SetParams<LitC_BP_Cube_PS::MaterialCB>(LitC_BP_Cube_PS::CB_SLOT_mat, opaqueCB);
    m_opaqueMat->SetTexture(LitC_BP_Cube_PS::TEX_SLOT_cubemap, m_cubemap.GetTexture());

    m_transparentMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Transparent");
    m_transparentMat->SetParams<LitC_BP_Transparent_PS::MaterialCB>(LitC_BP_Transparent_PS::CB_SLOT_mat, transparentCB);
    m_transparentMat->SetTexture(LitC_BP_Transparent_PS::TEX_SLOT_cubemap, m_cubemap.GetTexture());
    m_transparentMat->SetTexture(LitC_BP_Transparent_PS::TEX_SLOT_framebufferTex, m_mainPass.fb.gBuffer[0]);
    m_transparentMat->SetTexture(LitC_BP_Transparent_PS::TEX_SLOT_framebufferDepth, m_mainPass.fb.depthBuffer);
}

void Transparency_Scene::InitObjects()
{
    Mesh sphere = GeometryFactory::Instance()->BuildUVSphere(20, 20);
    Mesh cube = GeometryFactory::Instance()->BuildCube();
    Mesh donut = GeometryFactory::Instance()->BuildDonut(1.0f, 0.35f, 24, 12);

    Object* floor = CreateObject(DYNAMIC);
    floor->mesh = cube;
    floor->mat = m_floorMat.get();
    floor->transform.SetPosition(glm::vec3(0.0f, -2.0f, 0.f));
    floor->transform.SetScale(glm::vec3(25.0f, 0.2f, 25.0f));
    floor->drawMask = OPAQUE;

    Object* back = CreateObject(DYNAMIC);
    back->mesh = cube;
    back->mat = m_opaqueMat.get();
    back->transform.SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
    back->transform.SetScale(glm::vec3(1.5f));
    back->drawMask = OPAQUE;

    Object* metal = CreateObject(DYNAMIC);
    metal->mesh = donut;
    metal->mat = m_transparentMat.get();
    metal->transform.SetPosition(glm::vec3(2.5f, 0.0f, 1.0f));
    metal->drawMask = TRANSPARENT;

    Object* tinted = CreateObject(DYNAMIC);
    tinted->mesh = cube;
    tinted->mat = m_transparentMat.get();
    tinted->transform.SetPosition(glm::vec3(-2.0f, 0.0f, 0.0f));
    tinted->drawMask = TRANSPARENT;

    Object* glass = CreateObject(DYNAMIC);
    glass->mesh = sphere;
    glass->mat = m_transparentMat.get();
    glass->transform.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));
    glass->drawMask = TRANSPARENT;

    skyboxObject = new Object();
    m_skyboxMat = ShaderFactory::Instance()->CreateMaterial("UnlitSkybox");
    m_skyboxMat->SetTexture(UnlitSkybox_PS::TEX_SLOT_albedo, m_cubemap.GetTexture());

    skyboxObject->mat = m_skyboxMat.get();
    skyboxObject->mesh = GeometryFactory::Instance()->BuildSkybox();
    skyboxObject->transform.SetScale(50.0f);
}

void Transparency_Scene::OnInit(Device* _pDevice)
{
    m_presentTarget = _pDevice->GetDefaultFrameBuffer().gBuffer[0];
    
    InitLightScene();

    m_cubemap.SetTexture("Skybox", "Assets/Textures/Cubemap/FishPond/posz.jpg", "Assets/Textures/Cubemap/FishPond/negz.jpg",
        "Assets/Textures/Cubemap/FishPond/negx.jpg", "Assets/Textures/Cubemap/FishPond/posx.jpg",
        "Assets/Textures/Cubemap/FishPond/posy.jpg", "Assets/Textures/Cubemap/FishPond/negy.jpg");

    InitPassDesc(_pDevice);
    InitMaterials();
    InitObjects();

    _pDevice->SetLightData(m_lightData);



    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));

    m_toggleSkybox = true;
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

    m_camera.FillData(&m_refractionPass.passData);    
    m_postProcPass.passData.deltaTime = m_deltaTime;
    m_postProcPass.passData.totalTime = m_totalTime;
    m_refractionPass.passData.deltaTime = m_deltaTime;
    m_refractionPass.passData.totalTime = m_totalTime;

    _pDevice->BeginPass(m_mainPass);
        DrawObjects(_pDevice, m_camera.GetFrustum(), OPAQUE);
    _pDevice->EndPass();

    if (m_toggleSkybox)
    {
        m_camera.FillData(&m_skyboxPass.passData);
        _pDevice->BeginPass(m_skyboxPass);
        _pDevice->RegisterCommand(skyboxObject->mesh, skyboxObject->transform.GetMatrix(), skyboxObject->mat);
        _pDevice->EndPass();
    }

   _pDevice->BeginPass(m_depthCopyPass);
       _pDevice->Dispatch(m_depthCopyMat.get(), (uint32_t)ceil(WIDTH / 32.0f), (uint32_t)ceil(HEIGHT / 32.0f), 1);
   _pDevice->EndPass();

    _pDevice->BeginPass(m_refractionPass);
        DrawObjects(_pDevice, m_camera.GetFrustum(), TRANSPARENT);
    _pDevice->EndPass();

   _pDevice->BeginPass(m_postProcPass);
       _pDevice->Dispatch(m_postProcMat.get(), (uint32_t)ceil(WIDTH / 32.0f), (uint32_t)ceil(HEIGHT / 32.0f), 1);
   _pDevice->EndPass();


}


