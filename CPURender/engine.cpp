#include "engine.h"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <glm/mat4x4.hpp>

#include "Datas/Texture.h"


void Engine::init(Texture* _frameBuffer)
{
    printf("Init engine \n");
    
    m_device.SetRenderTarget(_frameBuffer);
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        
    }
}

void Engine::renderFrame()
{
    
}

void Engine::destroy()
{
    printf("Killing engine \n");
}
