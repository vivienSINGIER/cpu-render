#include "SceneBillingPhong.h"
#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

#include "../Shaders/GraphicsShaders/LitC_BP_Cube.hpp"

#include "../Factories/BufferFactory.h"

#include "../Datas/SkinnedObject.h"

void SceneBillingPhong::OnInit(Device* _pDevice)
{
    m_lightData.lights[0] = MakeDirectionalLight(normalize(vec3( 1.0f, -1.0f, 1.0f )), 1.6f, Color::WHITE);
    m_lightData.ambient = 0.4f;

    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_presentTarget = m_mainPass.fb.gBuffer[0];

    _pDevice->SetLightData(m_lightData);
    _pDevice->SetAutoExposure(false);
    _pDevice->SetExposure(1.0f);

    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));

    Handle diff = TextureFactory::Instance()->CreateTexture("dark_rock_diffuse", "Assets/Textures/DarkRock/diffuse.png", true);
    Handle ambient = TextureFactory::Instance()->CreateTexture("dark_rock_ambient", "Assets/Textures/DarkRock/ambient.png");
    Handle normal = TextureFactory::Instance()->CreateTexture("dark_rock_normal", "Assets/Textures/DarkRock/normal.png");
    Handle roughness = TextureFactory::Instance()->CreateTexture("dark_rock_roughness", "Assets/Textures/DarkRock/roughness.png");
    Handle displacement = TextureFactory::Instance()->CreateTexture("dark_rock_displacement", "Assets/Textures/DarkRock/displacement.png");
    Handle metalness = TextureFactory::Instance()->CreateTexture("dark_rock_metalness", "Assets/Textures/DarkRock/arm.png");

    LitT_BP_PS::MaterialCB cb = {
        {1.0f, 1.0f, 1.0f, 1.0f},
            Lighting::g_fresnelR0Templates[PLASTIC],
                0.9f, 0.02f, 0.5f
    };

    Material rock = ShaderFactory::Instance()->CreateMaterial("LitT_BP");
    rock = ShaderFactory::Instance()->CreateMaterial("LitT_BP");
    rock->SetParams<LitT_BP_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);
    rock->SetTexture(LitT_BP_PS::TEX_SLOT_diffuse, diff);
    rock->SetTexture(LitT_BP_PS::TEX_SLOT_ambient, ambient);
    rock->SetTexture(LitT_BP_PS::TEX_SLOT_normal, normal);
    rock->SetTexture(LitT_BP_PS::TEX_SLOT_roughness, roughness);
    rock->SetTexture(LitT_BP_PS::TEX_SLOT_displacement, displacement);
    rock->SetTexture(LitT_BP_PS::TEX_SLOT_metal, metalness);

    materials["Rock"] = std::move(rock);
    
    Mesh m = GeometryFactory::Instance()->BuildDonut(1.5f, 0.5f, 30, 20);
    Object* floor = CreateObject(DYNAMIC);
    floor->mesh = m;
    floor->mat = materials["Rock"].get();
    floor->transform.SetPosition(glm::vec3(0.0f, 0.f, 0.f));
}

void SceneBillingPhong::OnHandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
        return;

    glm::vec3& light_dir = m_lightData.lights[0].light.direction;

    glm::vec3 add = {};
    if (event.key.key == SDLK_RIGHT)
        add.x -= 1.f;

    if (event.key.key == SDLK_LEFT)
        add.x += 1.f;

    if (event.key.key == SDLK_UP)
        add.y += 1.f;

    if (event.key.key == SDLK_DOWN)
        add.y -= 1.f;

    light_dir += add * 10.f * m_deltaTime;
    glm::normalize(light_dir);
    lights_dirty = true;
}

void SceneBillingPhong::OnUpdate(float dt)
{
    //ParticleManager::Instance()->Update(dt);
}

void SceneBillingPhong::OnDraw(Device* _pDevice)
{
    if (lights_dirty)
    {
        lights_dirty = false;
        _pDevice->SetLightData(m_lightData);
    }

    m_camera.FillData(&m_mainPass.passData);
    m_mainPass.passData.deltaTime = m_deltaTime;
    m_mainPass.passData.totalTime = m_totalTime;

    _pDevice->BeginPass(m_mainPass);
    DrawObjects(_pDevice, m_camera.GetFrustum());
    _pDevice->EndPass();

}
