#include "engine.h"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <glm/mat4x4.hpp>


struct PixelData {
    int width, height;
    std::vector<uint32_t> imageData;
};

static PixelData doomFace;

void Engine::init()
{
    printf("Init engine \n");
    static int nbrChannel;
    unsigned char* data = stbi_load("Assets/doomguy.png", &doomFace.width, &doomFace.height, &nbrChannel, 4);
    
    doomFace.imageData.resize(doomFace.width * doomFace.height);

    //We copy the texture data in a format we can use
    for (int i = 0; i < doomFace.width; ++i)
    {
        for (int j = 0; j < doomFace.height; ++j)
        {
            uint32_t colorValue = 0;
            int linearized2D = i + j * doomFace.width;
            colorValue = colorValue | (uint32_t(data[0 +  linearized2D * nbrChannel]) << 16);//First channel is Red, highest weight bits
            colorValue = colorValue | (uint32_t(data[1 +  linearized2D * nbrChannel]) << 8); //Then green
            colorValue = colorValue | (uint32_t(data[2 +  linearized2D * nbrChannel]) << 0); //Finally blue
            doomFace.imageData[linearized2D] = colorValue;
        }
    }
    free(data);
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
    }
}

void Engine::renderFrame(Texture& _tex)
{

    _tex.Clear(Color::BLACK);

    //draws the doomguy face texture at the center of the screen
    for (int x = 0; x < doomFace.width; x++)
    {
        for (int y = 0; y < doomFace.height; y++) 
        {
            int posX = (WIDTH / 2 - doomFace.width/2) + x;
            int posY = (HEIGHT / 2 - doomFace.height/2) + y;
            frameBufferOut.put_pixel(posX, posY, doomFace.imageData[x + y*doomFace.width]);
        }
    }
}

void Engine::destroy()
{
    doomFace.imageData = std::vector<uint32_t>();
    printf("Killing engine \n");
}
