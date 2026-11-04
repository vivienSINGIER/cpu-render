#include "SnowglobeScene.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

enum DrawMask {
    OPAQUE = 0b00000001,
    TRANSPARENT = 0b00000010,
};

void SnowglobeScene::InitLightScene()
{
    m_lightData.lights[0] = MakeDirectionalLight({ 1.0f, -1.0f, 1.0f }, 1.5f, Color::WHITE);
    m_lightData.ambient = 0.2f;
}

void SnowglobeScene::InitPassDesc(Device* _pDevice)
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

void SnowglobeScene::InitMaterials()
{
    LitC_BP_Cube_PS::MaterialCB floorCB = { {0.5f, 0.5f, 0.1f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.8f, 0.0f, 0.0f, 1.7f, false };
    LitC_BP_Cube_PS::MaterialCB opaqueCB = { {0.7f, 0.7f, 0.7f, 1.0f}, Lighting::g_fresnelR0Templates[PLASTIC], 0.5f, 0.0f, 0.0f, 1.7f, false };
    LitC_BP_Transparent_PS::MaterialCB transparentCB = { {1.0f, 1.0f, 1.0f, 1.0f}, Lighting::g_fresnelR0Templates[CRYSTAL], 0.2f, 0.5f, 0.9f, 1.05f, true, {WIDTH, HEIGHT} };

    m_postProcMat = ShaderFactory::Instance()->CreateComputeMaterial("FBFusion");
    m_postProcMat->SetRWTexture(FB_Fusion_CS::UAV_SLOT_target1, m_mainPass.fb.gBuffer[0]);
    m_postProcMat->SetRWTexture(FB_Fusion_CS::UAV_SLOT_target2, m_refractionPass.fb.gBuffer[0]);
    m_postProcMat->SetRWTexture(FB_Fusion_CS::UAV_SLOT_target1db, m_mainPass.fb.depthBuffer);
    m_postProcMat->SetRWTexture(FB_Fusion_CS::UAV_SLOT_target2db, m_refractionPass.fb.depthBuffer);

    m_depthCopyMat = ShaderFactory::Instance()->CreateComputeMaterial("TexCopy");
    m_depthCopyMat->SetRWTexture(TexCopy_CS::UAV_SLOT_source, m_mainPass.fb.depthBuffer);
    m_depthCopyMat->SetRWTexture(TexCopy_CS::UAV_SLOT_target, m_refractionPass.fb.depthBuffer);

    m_snowMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Cube");
    m_snowMat->SetParams<LitC_BP_Cube_PS::MaterialCB>(LitC_BP_Cube_PS::CB_SLOT_mat, opaqueCB);
    m_snowMat->SetTexture(LitC_BP_Cube_PS::TEX_SLOT_cubemap, m_cubemap.GetTexture());

    m_floorMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Cube");
    m_floorMat->SetParams<LitC_BP_Cube_PS::MaterialCB>(LitC_BP_Cube_PS::CB_SLOT_mat, floorCB);
    m_floorMat->SetTexture(LitC_BP_Cube_PS::TEX_SLOT_cubemap, m_cubemap.GetTexture());

    m_transparentMat = ShaderFactory::Instance()->CreateMaterial("LitC_BP_Transparent");
    m_transparentMat->SetParams<LitC_BP_Transparent_PS::MaterialCB>(LitC_BP_Transparent_PS::CB_SLOT_mat, transparentCB);
    m_transparentMat->SetTexture(LitC_BP_Transparent_PS::TEX_SLOT_cubemap, m_cubemap.GetTexture());
    m_transparentMat->SetTexture(LitC_BP_Transparent_PS::TEX_SLOT_framebufferTex, m_mainPass.fb.gBuffer[0]);
    m_transparentMat->SetTexture(LitC_BP_Transparent_PS::TEX_SLOT_framebufferDepth, m_mainPass.fb.depthBuffer);
}

void SnowglobeScene::InitObjects()
{
    Mesh sphere = GeometryFactory::Instance()->BuildUVSphere(16, 16);
    Mesh box = GeometryFactory::Instance()->BuildCube();
    Mesh cylinder = GeometryFactory::Instance()->BuildCylinder(16);

    Object* glass = CreateObject(DYNAMIC);
    glass->mesh = sphere;
    glass->mat = m_transparentMat.get();
    glass->transform.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    glass->transform.SetScale(glm::vec3(20.0f, 20.0f, 20.0f));
    glass->drawMask = TRANSPARENT;

    skyboxObject = new Object();
    m_skyboxMat = ShaderFactory::Instance()->CreateMaterial("UnlitSkybox");
    m_skyboxMat->SetTexture(UnlitSkybox_PS::TEX_SLOT_albedo, m_cubemap.GetTexture());

    skyboxObject->mat = m_skyboxMat.get();
    skyboxObject->mesh = GeometryFactory::Instance()->BuildSkybox();
    skyboxObject->transform.SetScale(500.0f);

    Object* SnowmanObject = CreateObject(DYNAMIC);
    SnowmanObject->mesh = GeometryFactory::Instance()->LoadMesh("Assets/Obj/Snowman/snowman.obj");
    SnowmanObject->mat = m_snowMat.get();
    SnowmanObject->transform.SetPosition(glm::vec3(8.0f, -5.0f, 8.0f));
    SnowmanObject->transform.SetYPR(glm::vec3(0.0f, 3.1415/4.0f, 0.0f));
    SnowmanObject->transform.SetScale(glm::vec3(2.0f, 2.0f, 2.0f));
    SnowmanObject->drawMask = OPAQUE;

    Object* Ground = CreateObject(DYNAMIC);
    Ground->mesh = cylinder;
    Ground->mat = m_snowMat.get();
    Ground->transform.SetPosition(glm::vec3(0.0f, -10.0f, 0.0f));
    Ground->transform.SetScale(glm::vec3(35.0f, 10.0f, 35.0f));
    Ground->drawMask = OPAQUE;

    Object* Base = CreateObject(DYNAMIC);
    Base->mesh = cylinder;
    Base->mat = m_floorMat.get();
    Base->transform.SetPosition(glm::vec3(0.0f, -15.0f, 0.0f));
    Base->transform.SetScale(glm::vec3(45.0f, 10.0f, 45.0f));
    Base->drawMask = OPAQUE;
}

void SnowglobeScene::OnInit(Device* _pDevice)
{
    m_presentTarget = _pDevice->GetDefaultFrameBuffer().gBuffer[0];

    InitLightScene();

    m_cubemap.SetTexture("Skybox",
        "Assets/Textures/Cubemap/StylizedSky/pz.png", "Assets/Textures/Cubemap/StylizedSky/nz.png",
        "Assets/Textures/Cubemap/StylizedSky/nx.png", "Assets/Textures/Cubemap/StylizedSky/px.png",
        "Assets/Textures/Cubemap/StylizedSky/py.png", "Assets/Textures/Cubemap/StylizedSky/ny.png");

    InitPassDesc(_pDevice);
    InitMaterials();
    InitObjects();

    _pDevice->SetLightData(m_lightData);

    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));

    m_toggleSkybox = true;
}

void SnowglobeScene::CycleBlendMode(int _step)
{
    m_blendIndex = (m_blendIndex + _step + Blending::COUNT) % Blending::COUNT;
    m_mainPass.state.blendMode = (Blending::BlendMode)m_blendIndex;
}

void SnowglobeScene::OnHandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
        return;

}

void SnowglobeScene::OnUpdate(float dt)
{
}

void SnowglobeScene::OnDraw(Device* _pDevice)
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


