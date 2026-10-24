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
    
    Object* o1 = mainScene.CreateObject(DYNAMIC);
    o1->transform.SetPosition(glm::vec3(0, 0, 0));

    o1->mesh = GeometryFactory::Instance()->BuildCube();
    o1->mat = ShaderFactory::Instance()->CreateMaterial("UnlitC");
    o1->mat->SetParams<UnlitPS::UnlitColor>(CB_CUSTOM, { Color::GREEN });

   /* Object* o2 = mainScene.CreateObject(DYNAMIC);
    o2->transform.SetPosition(glm::vec3(0, 0, 0));

    o2->mesh = GeometryFactory::Instance()->BuildUVSphere(10, 10);
    o2->mat = ShaderFactory::Instance()->CreateMaterial("UnlitC");
    o2->mat->SetParams<UnlitPS::UnlitColor>(CB_CUSTOM, { Color::RED });*/

    printf("Init engine \n");
    
    m_device.SetRenderTarget(_frameBuffer);
    m_device.SetMainCamera(&m_camera);
    
    m_camera.SetAspectRatio((float)WIDTH / (float)HEIGHT);
    m_camTransform.SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
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

    //std::cout << "FPS : " << 1 / dt << "\n";

    // Update //
    UpdateCamera(glm::min(dt, MAX_DELTA_TIME));
    if (m_camTransform.IsWorldDirty())
    {
        m_camera.SetWorld(m_camTransform.GetMatrix());
    }

    //m_test->transform.AddYPR(glm::vec3(dt, 0, 0));
    mainScene.Update(dt);

    // Draw //
    m_device.BeginDraw();

    mainScene.Draw(&m_device, m_camera.GetFrustum());
    m_device.Draw();

    m_device.EndDraw();
}

void Engine::destroy()
{
    printf("Killing engine \n");
}
