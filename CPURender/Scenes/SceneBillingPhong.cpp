#include "SceneBillingPhong.h"
#include "../Core/Device.h"

#include <SDL3/SDL_keyboard.h>

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Factories/GeometryFactory.h"
#include "../Shaders/Shaders.h"
#include "../Datas/Lights/LightHelpers.hpp"

namespace
{
    // Textures paths are relative to Assets/Textures/<folder>/, nullptr = slot left empty (shader falls back to the CB values)
    struct MaterialDesc
    {
        const char* folder;
        const char* diffuse;
        const char* normal;
        const char* arm;          // r = AO, g = roughness, b = metalness
        const char* displacement;
        float heightScale;
        float roughness;          // used only without arm
        float metalness;          // used only without arm
    };

    const MaterialDesc g_materials[] =
    {
        { "DarkRock",       "diffuse.png", "normal.png", "arm.png", "displacement.png", 0.02f, 0.9f, 0.0f },
        { "CherryWood",     "diffuse.png", "normal.png", "ARM.png", "displacement.png", 0.02f, 0.6f, 0.0f },
        { "Granite",        "diff.png",    "nor.png",    "arm.png", nullptr,            0.0f,  0.6f, 0.0f },
        { "Slate",          "diff.png",    "nor.png",    "arm.png", nullptr,            0.0f,  0.7f, 0.0f },
        { "Laminate",       "diff.png",    "nor.png",    "arm.png", nullptr,            0.0f,  0.5f, 0.0f },
        { "Linoleum",       "diff.png",    "nor.png",    "arm.png", nullptr,            0.0f,  0.5f, 0.0f },
        { "BlueMetalPlate", "diff.png",    "nor.png",    "arm.png", nullptr,            0.0f,  0.4f, 1.0f },
        { "RustyMetal",     "diff.png",    "nor.png",    "arm.png", nullptr,            0.0f,  0.6f, 1.0f },
        { "Brick",          "diff.png",    "nor.png",    "arm.png", "disp.png",         0.04f, 0.8f, 0.0f },
    };
    constexpr int MATERIAL_COUNT = (int)(sizeof(g_materials) / sizeof(g_materials[0]));

    constexpr float LIGHT_ROT_SPEED = glm::radians(90.0f);  // per second
    constexpr float LIGHT_MAX_PITCH = glm::radians(89.0f);  // avoid the pole where yaw degenerates

    int Wrap(int _i, int _count) { return ((_i % _count) + _count) % _count; }

    Handle LoadTex(MaterialDesc const& _d, const char* _file, bool _isSrgb)
    {
        if (_file == nullptr)
            return Handle();

        std::string path = std::string("Assets/Textures/") + _d.folder + "/" + _file;
        return TextureFactory::Instance()->CreateTexture(std::string(_d.folder) + "_" + _file, path, _isSrgb);
    }
}

void SceneBlinnPhong::OnInit(Device* _pDevice)
{
    m_lightYaw = glm::atan(-1.0f, -1.0f);
    m_lightPitch = glm::asin(1.0f / glm::sqrt(3.0f));
    m_lightData.lights[0] = MakeDirectionalLight(vec3(0.0f, -1.0f, 0.0f), 1.6f, Color::WHITE);
    m_lightData.ambient = 0.4f;
    UpdateLightDirection();

    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();
    m_presentTarget = m_mainPass.fb.gBuffer[0];

    _pDevice->SetLightData(m_lightData);
    _pDevice->SetAutoExposure(false);
    _pDevice->SetExposure(1.0f);

    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));

    m_materials.resize(MATERIAL_COUNT);

    struct MeshEntry { Mesh mesh; float scale; };
    MeshEntry meshes[] =
    {
        { GeometryFactory::Instance()->BuildDonut(1.5f, 0.5f, 30, 20), 1.0f },
        { GeometryFactory::Instance()->BuildUVSphere(32, 16),          2.0f },
        { GeometryFactory::Instance()->BuildIcosphere(3),              2.0f },
        { GeometryFactory::Instance()->BuildCube(),                    2.5f },
    };

    for (MeshEntry const& e : meshes)
    {
        Object* o = CreateObject(DYNAMIC);
        o->mesh = e.mesh;
        o->transform.SetPosition(glm::vec3(0.0f));
        o->transform.SetScale(glm::vec3(e.scale));
        o->drawMask = 0;
        m_meshObjects.push_back(o);
    }

    SelectMaterial(0);
    SelectMesh(0);
}

MaterialData* SceneBlinnPhong::GetOrCreateMaterial(int _index)
{
    if (m_materials[_index])
        return m_materials[_index].get();

    MaterialDesc const& d = g_materials[_index];

    LitT_BP_PS::MaterialCB cb = {
        {1.0f, 1.0f, 1.0f, 1.0f},
            Lighting::g_fresnelR0Templates[PLASTIC],
                d.roughness, d.heightScale, d.metalness
    };

    Material mat = ShaderFactory::Instance()->CreateMaterial("LitT_BP");
    mat->SetParams<LitT_BP_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);

    if (d.diffuse)      mat->SetTexture(LitT_BP_PS::TEX_SLOT_diffuse,      LoadTex(d, d.diffuse, true));
    if (d.normal)       mat->SetTexture(LitT_BP_PS::TEX_SLOT_normal,       LoadTex(d, d.normal, false));
    if (d.arm)          mat->SetTexture(LitT_BP_PS::TEX_SLOT_arm,          LoadTex(d, d.arm, false));
    if (d.displacement) mat->SetTexture(LitT_BP_PS::TEX_SLOT_displacement, LoadTex(d, d.displacement, false));

    m_materials[_index] = std::move(mat);
    return m_materials[_index].get();
}

void SceneBlinnPhong::SelectMaterial(int _index)
{
    m_materialIndex = Wrap(_index, MATERIAL_COUNT);
    MaterialData* mat = GetOrCreateMaterial(m_materialIndex);

    for (Object* o : m_meshObjects)
        o->mat = mat;

    std::cout << "Material : " << g_materials[m_materialIndex].folder << "\n";
}

void SceneBlinnPhong::SelectMesh(int _index)
{
    m_meshIndex = Wrap(_index, (int)m_meshObjects.size());

    for (size_t i = 0; i < m_meshObjects.size(); i++)
        m_meshObjects[i]->drawMask = (i == (size_t)m_meshIndex) ? 0b1111'1111 : 0;
}

void SceneBlinnPhong::UpdateLightDirection()
{
    glm::vec3 toLight = {
        glm::cos(m_lightPitch) * glm::sin(m_lightYaw),
        glm::sin(m_lightPitch),
        glm::cos(m_lightPitch) * glm::cos(m_lightYaw)
    };
    m_lightData.lights[0].light.direction = -toLight;
    lights_dirty = true;
}

void SceneBlinnPhong::OnHandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat)
        return;

    switch (event.key.key)
    {
    case SDLK_INSERT: SelectMaterial(m_materialIndex + 1); break;
    case SDLK_DELETE: SelectMaterial(m_materialIndex - 1); break;
    case SDLK_HOME:   SelectMesh(m_meshIndex + 1);         break;
    case SDLK_END:    SelectMesh(m_meshIndex - 1);         break;
    default: break;
    }
}

void SceneBlinnPhong::OnUpdate(float dt)
{
    bool const* keys = SDL_GetKeyboardState(nullptr);

    float yawInput   = (keys[SDL_SCANCODE_LEFT] ? 1.0f : 0.0f) - (keys[SDL_SCANCODE_RIGHT] ? 1.0f : 0.0f);
    float pitchInput = (keys[SDL_SCANCODE_UP]   ? 1.0f : 0.0f) - (keys[SDL_SCANCODE_DOWN]  ? 1.0f : 0.0f);

    if (yawInput == 0.0f && pitchInput == 0.0f)
        return;

    m_lightYaw = glm::mod(m_lightYaw + yawInput * LIGHT_ROT_SPEED * dt, glm::two_pi<float>());
    m_lightPitch = glm::clamp(m_lightPitch + pitchInput * LIGHT_ROT_SPEED * dt, -LIGHT_MAX_PITCH, LIGHT_MAX_PITCH);
    UpdateLightDirection();
}

void SceneBlinnPhong::OnDraw(Device* _pDevice)
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
