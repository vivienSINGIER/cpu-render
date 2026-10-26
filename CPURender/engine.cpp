#include "engine.h"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_timer.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <ostream>

#include "stb_image.h"
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>

#include "Datas/Texture.h"
#include "Factories/GeometryFactory.h"
#include "Shaders/Shaders.h"
#include "Factories/ShaderFactory.h"
#include "Factories/TextureFactory.h"

#include "Octree.h"

namespace
{
    constexpr float CAM_MOVE_SPEED = 3.0f;          // world units per second
    constexpr float CAM_KEY_ROT_SPEED = 1.5f;       // radians per second
    constexpr float CAM_MOUSE_SENSITIVITY = 0.005f; // radians per pixel
    constexpr float CAM_MAX_PITCH = glm::radians(89.0f);
    constexpr float MAX_DELTA_TIME = 0.1f;          // avoids a big jump after a hitch

    const glm::vec3 WORLD_UP = { 0.0f, 1.0f, 0.0f };
}

void Engine::RotateCamera(float _yaw, float _pitch)
{
    m_camYaw += _yaw;
    m_camPitch = glm::clamp(m_camPitch + _pitch, -CAM_MAX_PITCH, CAM_MAX_PITCH);
    
    glm::quat yaw = glm::angleAxis(m_camYaw, WORLD_UP);
    glm::quat pitch = glm::angleAxis(m_camPitch, glm::vec3(1.0f, 0.0f, 0.0f));

    m_camTransform.SetRotationQuaternion(glm::normalize(yaw * pitch));
}

void Engine::InitLightScene()
{
    m_lightData.lights[0] = MakeDirectionalLight({1.0f, -1.0f, 1.0f}, 0.8f);
    m_lightData.ambient = 0.4f;
}

void Engine::UpdateCamera(float _dt)
{
    bool const* keys = SDL_GetKeyboardState(nullptr);

    glm::vec3 move = { 0.0f, 0.0f, 0.0f };
    if (keys[SDL_SCANCODE_W])     move += m_camTransform.GetForward();
    if (keys[SDL_SCANCODE_S])     move -= m_camTransform.GetForward();
    if (keys[SDL_SCANCODE_D])     move += m_camTransform.GetRight();
    if (keys[SDL_SCANCODE_A])     move -= m_camTransform.GetRight();
    if (keys[SDL_SCANCODE_SPACE]) move += WORLD_UP;
    if (keys[SDL_SCANCODE_LCTRL]) move -= WORLD_UP;
    
    if (glm::dot(move, move) > 0.0f)
        m_camTransform.Move(glm::normalize(move), CAM_MOVE_SPEED * _dt);

    float yaw = 0.0f;
    float pitch = 0.0f;
    if (keys[SDL_SCANCODE_LEFT])  yaw -= 1.0f;
    if (keys[SDL_SCANCODE_RIGHT]) yaw += 1.0f;
    if (keys[SDL_SCANCODE_UP])    pitch -= 1.0f;
    if (keys[SDL_SCANCODE_DOWN])  pitch += 1.0f;

    if (yaw != 0.0f || pitch != 0.0f)
        RotateCamera(yaw * CAM_KEY_ROT_SPEED * _dt, pitch * CAM_KEY_ROT_SPEED * _dt);
}

void Engine::init(Texture* _frameBuffer)
{
    srand(time(nullptr));
    printf("Init engine \n");
    
    InitLightScene();
    
    m_mainPass.fb = m_device.GetDefaultFrameBuffer();
    
    m_shadowPass.fb = m_device.CreateFrameBuffer(512, 512, 0, true);
    m_shadowPass.state.cullMode = CullMode::FRONT;
    
    m_postProcPass.isCompute = true;
    
    m_device.SetRenderTarget(_frameBuffer);
    m_device.SetLightData(m_lightData);
    
    m_camera.SetAspectRatio((float)WIDTH / (float)HEIGHT);
    m_camera.SetShadowDistance(25.0f);
    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -10.f));

    Handle diff = TextureFactory::Instance()->CreateTexture("dark_rock_diffuse", "Assets/Textures/DarkRock/diffuse.png");
    Handle ambient = TextureFactory::Instance()->CreateTexture("dark_rock_ambient", "Assets/Textures/DarkRock/ambient.png");
    Handle normal = TextureFactory::Instance()->CreateTexture("dark_rock_normal", "Assets/Textures/DarkRock/normal.png");
    Handle roughness = TextureFactory::Instance()->CreateTexture("dark_rock_roughness", "Assets/Textures/DarkRock/roughness.png");
    Handle displacement = TextureFactory::Instance()->CreateTexture("dark_rock_displacement", "Assets/Textures/DarkRock/displacement.png");
    Handle metalness = TextureFactory::Instance()->CreateTexture("dark_rock_metalness", "Assets/Textures/DarkRock/metalness.png");
    
    LitT_BP_PS::MaterialCB cb = {
        {1.0f, 1.0f, 1.0f, 1.0f}, 
            Lighting::g_fresnelR0Templates[PLASTIC], 
                0.5f, 0.02f, 0.8f
    };

    Mesh m = GeometryFactory::Instance()->BuildUVSphere(10, 10, true);
    Mesh cube = GeometryFactory::Instance()->BuildCube();

    m_mat = ShaderFactory::Instance()->CreateMaterial("LitT_BP");
    m_mat->SetParams<LitT_BP_PS::MaterialCB>(LitT_BP_PS::CB_SLOT_mat, cb);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_diffuse, diff);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_ambient, ambient);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_normal, normal);
    
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_roughness, roughness);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_displacement, displacement);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_metal, metalness);
    m_mat->SetTexture(LitT_BP_PS::TEX_SLOT_shadow, m_shadowPass.fb.depthBuffer);
    
    m_groupDebugMat = ShaderFactory::Instance()->CreateComputeMaterial("GroupDebug");
    m_groupDebugMat->SetRWTexture(GroupDebug_CS::UAV_SLOT_target, m_mainPass.fb.gBuffer[0]);

    // for (int i = -1; i < 2; ++i)
    // {
    //     for (int j = -1; j < 2; ++j)
    //     {
    //         for (int k = -1; k < 2; ++k)
    //         {
                Object* o = mainScene.CreateObject(DYNAMIC);
                o->mesh = m;
                o->mat = m_mat.get();
                o->transform.SetPosition(glm::vec3(0, 0, 0));
    //         }
    //     }
    // }

    Object* floor = mainScene.CreateObject(DYNAMIC);
    floor->mesh = cube;
    floor->mat = m_mat.get();
    floor->transform.SetPosition(glm::vec3(0.0f, -5.0f, 0.f));
    floor->transform.SetScale(glm::vec3(25.0f, 0.2f, 25.0f));
    
    
    std::cout << "End Init" << std::endl;
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_MOUSE_MOTION && (event.motion.state & SDL_BUTTON_RMASK))
    {
        RotateCamera(event.motion.xrel * CAM_MOUSE_SENSITIVITY, event.motion.yrel * CAM_MOUSE_SENSITIVITY);
    }
}

void Engine::mainLoop()
{
    uint64_t now = SDL_GetPerformanceCounter();
    float dt = m_lastTime == 0 ? 0.0f : (float)(now - m_lastTime) / (float)SDL_GetPerformanceFrequency();
    m_lastTime = now;

    std::cout << "FPS : " << 1 / dt << "\n";

    // Update //
    UpdateCamera(glm::min(dt, MAX_DELTA_TIME));
    if (m_camTransform.IsWorldDirty())
    {
        m_camera.SetWorld(m_camTransform.GetMatrix());
    }

    mainScene.Update(dt);
    
    Frustum shadowFrustum;
    glm::mat4x4 lightSpace = Lighting::ComputeDirLightViewProj(m_shadowPass.view.view, m_camera.GetShadowProj(), 
                                        m_lightData.lights[0].light.direction, m_shadowPass.fb.width, shadowFrustum, 0.0f);
    
    m_camera.FillData(&m_mainPass.view);
    m_mainPass.view.deltaTime = dt;
    m_mainPass.view.totalTime = static_cast<float>(now);
    m_mainPass.view.lightSpace = lightSpace;
    
    m_camera.FillData(&m_shadowPass.view);
    m_shadowPass.view.viewProj = lightSpace;
    
    // Draw //

    m_device.BeginFrame();
    
    m_device.BeginPass(m_shadowPass);
        mainScene.Draw(&m_device, shadowFrustum);
    m_device.EndPass();
    
    m_device.BeginPass(m_mainPass);
        mainScene.Draw(&m_device, m_camera.GetFrustum());
    m_device.EndPass();
    
    // m_device.BeginPass(m_postProcPass);
    //     m_device.Dispatch(m_groupDebugMat.get(), 64, 64, 1);
    // m_device.EndPass();
    
    m_device.EndFrame();
    m_device.Present(m_mainPass.fb.gBuffer[0]);
}

void Engine::destroy()
{
    printf("Killing engine \n");
}
