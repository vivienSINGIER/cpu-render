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
    m_lightData.lights[0] = MakeDirectionalLight({1.0f, -1.0f, 1.0f}, 0.5f);
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
    printf("Init engine \n");
    
    InitLightScene();
    
    m_device.SetRenderTarget(_frameBuffer);
    m_device.SetMainCamera(&m_camera);
    m_device.SetLightData(m_lightData);
    
    m_camera.SetAspectRatio((float)WIDTH / (float)HEIGHT);
    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
    
    m_mesh = GeometryFactory::Instance()->BuildDonut(1.0f, 0.3f, 32, 32);
    
    Handle t = TextureFactory::Instance()->CreateTexture("doomGuy", "Assets/doomguy.png");
    
    LitC_BP_PS::MaterialCB cb = {
        {0.3f, 0.2f, 0.0f, 1.0f}, 
            Lighting::g_fresnelR0Templates[GOLD], 
                0.2f
    };
    
    m_material = ShaderFactory::Instance()->CreateMaterial("LitC_BP");
    m_material->SetParams<LitC_BP_PS::MaterialCB>(CB_CUSTOM, cb);
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_MOUSE_MOTION && (event.motion.state & SDL_BUTTON_RMASK))
    {
        RotateCamera(event.motion.xrel * CAM_MOUSE_SENSITIVITY, event.motion.yrel * CAM_MOUSE_SENSITIVITY);
    }
}

void Engine::renderFrame()
{
    uint64_t now = SDL_GetPerformanceCounter();
    float dt = m_lastTime == 0 ? 0.0f : (float)(now - m_lastTime) / (float)SDL_GetPerformanceFrequency();
    m_lastTime = now;

    std::cout << "FPS : " << 1 / dt << "\n";
    
    UpdateCamera(glm::min(dt, MAX_DELTA_TIME));
    
    if (m_camTransform.IsWorldDirty())
        m_camera.SetWorld(m_camTransform.GetMatrix());
    
    m_device.BeginDraw();
    
    m_device.RegisterCommand(m_mesh, m_meshTransform.GetMatrix(), m_material.get());
    m_device.Draw();
    
    m_device.EndDraw();
}

void Engine::destroy()
{
    printf("Killing engine \n");
}
