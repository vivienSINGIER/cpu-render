#include "engine.h"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

/* Doom fire variables and color palette */
#define FIRE_W WIDTH
#define FIRE_H HEIGHT

#define FIRE_PALETTE_SIZE 37

static uint8_t firePixels[FIRE_W * FIRE_H];

static const uint32_t fire_palette[FIRE_PALETTE_SIZE] = {
    0x070707, 0x1F0707, 0x2F0F07, 0x470F07, 0x571707, 0x671F07, 0x771F07,
    0x8F2707, 0x9F2F07, 0xAF3F07, 0xBF4707, 0xC74707, 0xDF4F07, 0xDF5707,
    0xDF5707, 0xD75F07, 0xD75F07, 0xD7670F, 0xCF6F0F, 0xCF770F, 0xCF7F0F,
    0xCF8717, 0xC78717, 0xC78F17, 0xC7971F, 0xBF9F1F, 0xBF9F1F, 0xBFA727,
    0xBFA727, 0xBFAF2F, 0xB7AF2F, 0xB7B72F, 0xB7B737, 0xCFCF6F, 0xDFDF9F,
    0xEFEFC7, 0xFFFFFF
};


struct PixelData {
    int width, height;
    std::vector<uint32_t> imageData;
};

static PixelData doomFace;

void resetFire()
{
    /* Fire initialization */
    for (int i = 0; i < FIRE_W * FIRE_H; i++) {
        firePixels[i] = 0;
    }

    /* Seed the fire bottom row with maximum intensity (this is the fire source) */
    for (int i = 0; i < FIRE_W; i++) {
        firePixels[(FIRE_H - 1) * FIRE_W + i] = FIRE_PALETTE_SIZE - 1;
    }
}

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
    resetFire();

    mainCam = Camera();
    mainCam.transform = { 1, 0, 0, 0,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1 };

    std::shared_ptr<Object> debugTriangle = std::make_shared<Object>();

    float red[3] = { 255, 0, 0 };
    float green[3] = { 0, 255, 0 };
    float blue[3] = { 0, 0, 255 };
    float white[3] = { 255, 255, 255 };

    Triangle t1;
    t1.v1 = {glm::vec3(-1.0f, 0.0f, 0.0f), { 255, 0, 0 } };
    t1.v2 = {glm::vec3(0.0f, 1.0f, 0.0f), { 0, 255, 0 } };
    t1.v3 = {glm::vec3(1.0f, 0.0f, 0.0f), { 0, 0, 255 } };

    debugTriangle->mesh.triangles.push_back(t1);
    debugTriangle->transform = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 2, 1 };

    objects.push_back(debugTriangle);
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        if (event.key.key == SDLK_SPACE)
        {
            mainCam.Translate({ 0, 1.0f, 0 });
        }
        if (event.key.key == SDLK_LCTRL)
        {
            mainCam.Translate({ 0, -1.0f, 0 });
        }
        if (event.key.key == SDLK_S)
        {
            mainCam.Translate({ 0, 0, -1.0f });
        }
        if (event.key.key == SDLK_Z)
        {
            mainCam.Translate({ 0, 0, 1.0f});
        }
        if (event.key.key == SDLK_Q)
        {
            mainCam.Translate({ 1.0f, 0, 0 });
        }
        if (event.key.key == SDLK_D)
        {
            mainCam.Translate({ -1.0f, 0, 0 });
        }
        if (event.key.key == SDLK_A)
        {
            mainCam.RotateYaw(3.1415f / 4.0f);
        }
        if (event.key.key == SDLK_E)
        {
            mainCam.RotateYaw(-3.1415f / 4.0f);
        }
    }
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
    //objects[0]->transform = glm::rotate(objects[0]->transform, -3.1415f / float(target_frame) * 2, glm::vec3(0, 1, 0));


    frameBufferOut.clear(0 | (0 << 8) | (0 << 16) | (0 << 24));

    glm::mat4x4 proj = glm::perspectiveFov(90.0f, 320.0f, 240.0f, 0.1f, 100.0f);
    //glm::mat4x4 proj = glm::perspective(75.0f, 4.0f / 3.0f, 0.1f, 100.f);

    glm::mat4x4 view = glm::inverse(mainCam.transform);
    glm::vec4 viewport = glm::vec4(0, 0, 320, 240);

    glm::mat4x4 modelView = view * objects[0]->transform;

    // To screen
    Triangle screenSpace = objects[0]->mesh.triangles[0];
    screenSpace.v1.pos = glm::project(screenSpace.v1.pos, modelView, proj, viewport);
    screenSpace.v2.pos = glm::project(screenSpace.v2.pos, modelView, proj, viewport);
    screenSpace.v3.pos = glm::project(screenSpace.v3.pos, modelView, proj, viewport);

    if(screenSpace.IsFront() == false) return; // For now, Clockwise

    for (uint32_t x = 0; x < FIRE_W; x++) {
        for (uint32_t y = 1; y < FIRE_H; y++) {

            uint32_t color = 0;
            if (screenSpace.IsInTriangle({ x, y }))
            {
                color = screenSpace.GetPointColor({ x, y });
                frameBufferOut.put_pixel(x, y, color);
            }
        }
    }
}

void Engine::destroy()
{
    doomFace.imageData = std::vector<uint32_t>();
    printf("Killing engine \n");
}
