#include "engine.hpp"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <ostream>

#include "stb_image.h"

void Engine::handleEvent(const SDL_Event& event)
{
    if (m_mainScene->m_running == false)
        return;

    m_mainScene->HandleEvent(event);
    
    if (m_mainScene->m_running == false)
    {
        SDL_Event quit = {};
        quit.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit);
    }
}

void Engine::mainLoop()
{
    uint64_t now = SDL_GetPerformanceCounter();
    float dt = m_lastTime == 0 ? 0.0f : (float)(now - m_lastTime) / (float)SDL_GetPerformanceFrequency();
    m_lastTime = now;

    // Update //
    m_mainScene->Update(dt);

    // Draw //
    m_device.BeginFrame();
        m_mainScene->Draw(&m_device);
    m_device.EndFrame();

    m_device.Present(m_mainScene->m_presentTarget);
}

void Engine::destroy()
{
    printf("Killing engine \n");

    delete m_mainScene;
    m_mainScene = nullptr;
}
