#include "PBR_Scene.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

void PBR_Scene::InitLightScene()
{
    m_lightData.lights[0] = MakePointLight({5.0f, 5.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    // m_lightData.lights[1] = MakePointLight({3.0f, 3.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    // m_lightData.lights[2] = MakePointLight({-3.0f, 3.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    // m_lightData.lights[3] = MakePointLight({-3.0f, -3.0f, -5.0f}, 10.0f, { 5.0f, 25.0f }, Color::WHITE);
    m_lightData.ambient = 0.03f;
}

void PBR_Scene::OnInit(Device* _pDevice)
{
    InitLightScene();
    
    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_presentTarget = m_mainPass.fb.gBuffer[0];
    
    m_shadowPass.fb = _pDevice->CreateFrameBuffer(1024, 1024, 0, true);
    m_shadowPass.state.cullMode = CullMode::FRONT;
    
    m_skyboxPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_skyboxPass.state.depthTest = false;
    m_skyboxPass.state.depthWrite = false;
    
    m_postProcPass.isCompute = true;
    
    _pDevice->SetLightData(m_lightData);
    
    m_camera.SetShadowDistance(25.0f);
    m_camTransform.SetPosition(glm::vec3(0.0f, 10.0f, -32.f));

    LoadCubemaps();

    Handle brdfLookup = TextureFactory::Instance()->CreateTexture("BRDF_Lookup", 512, 512);
    Texture::BakeBRDFLookup(TextureFactory::Instance()->GetTexture("BRDF_Lookup"));

    Mesh m = GeometryFactory::Instance()->BuildUVSphere(32, 32, true);

    const char* materials[] = { "BlueMetalPlate", "DarkRock", "Granite", "Laminate", "Linoleum", "RustyMetal", "Slate" };
    const int count = (int)std::size(materials);
    const float spacing = 2.5f;

    for (int i = 0; i < count; i++)
    {
        std::string name = materials[i];
        std::string dir = "Assets/Textures/" + name + "/";

        // DarkRock uses different file names than the other sets
        bool longNames = name == "DarkRock";
        Handle diff = TextureFactory::Instance()->CreateTexture(name + "_diffuse", dir + (longNames ? "diffuse.png" : "diff.png"), true);
        Handle normal = TextureFactory::Instance()->CreateTexture(name + "_normal", dir + (longNames ? "normal.png" : "nor.png"));
        Handle arm = TextureFactory::Instance()->CreateTexture(name + "_arm", dir + "arm.png");

        Material mat = ShaderFactory::Instance()->CreateMaterial("PBR");
        mat->SetTexture(PBR_PS::TEX_SLOT_brdfLUT, brdfLookup);
        mat->SetTexture(PBR_PS::TEX_SLOT_diffuse, diff);
        mat->SetTexture(PBR_PS::TEX_SLOT_normal, normal);
        mat->SetTexture(PBR_PS::TEX_SLOT_arm, arm);

        Object* o = CreateObject(DYNAMIC);
        o->mat = mat.get();
        o->mesh = m;
        o->transform.SetPosition(glm::vec3((i - (count - 1) * 0.5f) * spacing, 0.0f, 0.0f));

        m_mats.push_back(std::move(mat));
    }

    // textureless grid above the row : roughness along x, metalness along y
    const int gridSize = 8;
    const float gridBottom = 3.5f;

    for (int y = 0; y < gridSize; y++)
    {
        for (int x = 0; x < gridSize; x++)
        {
            float roughness = glm::max((float)x / (float)(gridSize - 1), 0.05f);
            float metalness = (float)y / (float)(gridSize - 1);

            Material mat = ShaderFactory::Instance()->CreateMaterial("PBR");
            mat->SetTexture(PBR_PS::TEX_SLOT_brdfLUT, brdfLookup);

            PBR_PS::MaterialCB cb = { {0.5f, 0.0f, 0.0f}, metalness, roughness, 1.0f };
            mat->SetParams<PBR_PS::MaterialCB>(PBR_PS::CB_SLOT_mat, cb);

            Object* o = CreateObject(DYNAMIC);
            o->mat = mat.get();
            o->mesh = m;
            o->transform.SetPosition(glm::vec3((x - (gridSize - 1) * 0.5f) * spacing, gridBottom + y * spacing, 0.0f));

            m_gridMats.push_back(std::move(mat));
        }
    }

    UpdateMaterial();

    SetSkybox(m_cubemaps[m_cubemapIndex]);
    ApplyCubemap(m_cubemapIndex);
}

void PBR_Scene::LoadCubemaps()
{
    // FishPond & Meadow use the posx/negx naming, the others front/back
    struct CubemapDesc { const char* name; bool posNeg; const char* ext; };
    const CubemapDesc descs[] = {
        { "FishPond", true,  "jpg" },
        { "Meadow",   true,  "jpg" },
        { "Coast",    false, "png" },
        { "Dawn",     false, "png" },
        { "studio",   false, "png" },
    };

    // order expected by Cubemap : front, back, left, right, up, down
    const char* posNegFaces[6] = { "posz", "negz", "negx", "posx", "posy", "negy" };
    const char* namedFaces[6]  = { "front", "back", "right", "left", "up", "down" };

    for (CubemapDesc const& d : descs)
    {
        std::string dir = std::string("Assets/Textures/Cubemap/") + d.name + "/";
        const char** faces = d.posNeg ? posNegFaces : namedFaces;

        std::string paths[6];
        for (int i = 0; i < 6; i++)
            paths[i] = dir + faces[i] + "." + d.ext;

        std::cout << "Loading cubemap " << d.name << "... ";
        m_cubemaps.push_back(Cubemap((std::string("Skybox_") + d.name).c_str(), paths[0].c_str(), paths[1].c_str(),
            paths[2].c_str(), paths[3].c_str(), paths[4].c_str(), paths[5].c_str(), true, 32));
        m_cubemapNames.push_back(d.name);
    }
}

void PBR_Scene::ApplyCubemap(int _index)
{
    int size = (int)m_cubemaps.size();
    m_cubemapIndex = (_index % size + size) % size;
    Cubemap& c = m_cubemaps[m_cubemapIndex];

    for (Material& mat : m_mats)
    {
        mat->SetTexture(PBR_PS::TEX_SLOT_irradience, c.GetIrradianceTexture());
        mat->SetTexture(PBR_PS::TEX_SLOT_prefilter, c.GetPrefilterTexture());
    }
    for (Material& mat : m_gridMats)
    {
        mat->SetTexture(PBR_PS::TEX_SLOT_irradience, c.GetIrradianceTexture());
        mat->SetTexture(PBR_PS::TEX_SLOT_prefilter, c.GetPrefilterTexture());
    }
    m_skyboxMat->SetTexture(UnlitSkybox_PS::TEX_SLOT_albedo, c.GetTexture());

    std::cout << "Cubemap : " << m_cubemapNames[m_cubemapIndex] << std::endl;
}

void PBR_Scene::UpdateMaterial()
{
    // roughness 0 makes the GGX lobe a dirac, so the highlight vanishes
    m_roughness = glm::clamp(m_roughness, 0.05f, 1.0f);
    m_metalness = glm::clamp(m_metalness, 0.0f, 1.0f);

    PBR_PS::MaterialCB cb = { {0.5f, 0.0f, 0.0f}, m_metalness, m_roughness, 1.0f };
    for (Material& mat : m_mats)
        mat->SetParams<PBR_PS::MaterialCB>(PBR_PS::CB_SLOT_mat, cb);

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

    if (event.key.key == SDLK_N) ApplyCubemap(m_cubemapIndex + 1);
    if (event.key.key == SDLK_B) ApplyCubemap(m_cubemapIndex - 1);
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

void PBR_Scene::SetSkybox(Cubemap _cubemap)
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

