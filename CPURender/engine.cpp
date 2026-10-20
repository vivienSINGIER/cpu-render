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

#define NEAR 0.1f
#define FAR 100.0f


void Engine::init()
{
    mainCam = Camera();
    mainCam.transform = glm::identity<glm::mat4x4>();

    std::shared_ptr<Object> debugTriangle = std::make_shared<Object>();
    std::shared_ptr<Object> debugTriangle2 = std::make_shared<Object>();
    std::shared_ptr<Object> debugTriangle3 = std::make_shared<Object>();

    float red[3] = { 255, 0, 0 };
    float green[3] = { 0, 255, 0 };
    float blue[3] = { 0, 0, 255 };
    float white[3] = { 255, 255, 255 };

    Triangle t1;
    t1.v1 = {glm::vec3(1.0f, 0.0f, 0.0f), { 0, 0, 255 } };
    t1.v2 = {glm::vec3(0.0f, 1.0f, 0.0f), { 0, 255, 0 } };
    t1.v3 = {glm::vec3(-1.0f, 0.0f, 0.0f), { 255, 0, 0 } };

    debugTriangle->mesh.triangles.push_back(t1);
    debugTriangle->transform = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 2, 1 };

    debugTriangle2->mesh.triangles.push_back(t1);
    debugTriangle2->transform = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0.5, 0, 2, 1 };

    objects.push_back(debugTriangle);
    objects.push_back(debugTriangle2);
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

void Engine::renderFrame(Texture& _tex)
{
    frameBufferOut.clear(0 | (0 << 8) | (0 << 16) | (0 << 24));

    objects[0]->transform = glm::rotate(objects[0]->transform, 3.1415f * 4.0f / (float)target_frame, {0, 1, 0});

    // Create Depth Buffer
    size_t pixelCount = sizeof(frameBufferOut.colorData) / sizeof(uint32_t);
    std::vector<float> depthBuffer = std::vector<float>(pixelCount);
    std::fill(depthBuffer.begin(), depthBuffer.end(), FAR);

    glm::mat4x4 proj = glm::perspectiveFov(M_PI/2.0f, 320.0f, 240.0f, NEAR, FAR);

    glm::mat4x4 view = glm::inverse(mainCam.transform);
    glm::vec4 viewport = glm::vec4(0, 0, 320, 240);

    size_t objCount = objects.size();
    for (int i = 0; i < objCount; i++)
    {
        glm::mat4x4 modelView = view * objects[i]->transform;

        size_t triCount = objects[i]->mesh.triangles.size();
        for (int j = 0; j < triCount; j++)
        {
            // To screen
            Triangle screenSpace = objects[i]->mesh.triangles[j];
            screenSpace.v1.pos = glm::project(screenSpace.v1.pos, modelView, proj, viewport);
            screenSpace.v2.pos = glm::project(screenSpace.v2.pos, modelView, proj, viewport);
            screenSpace.v3.pos = glm::project(screenSpace.v3.pos, modelView, proj, viewport);

            if (screenSpace.IsFront() == false) continue; // For now, Clockwise

            glm::vec4 box = screenSpace.GetBoundingBox();

            for (uint32_t x = box.x; x < box.z; x++) {
                for (uint32_t y = box.y; y < box.w; y++) {
                    uint32_t color = 0;
                    if (screenSpace.IsInTriangle({ x, y }))
                    {
                        glm::vec3 bary = screenSpace.BarycentricWeight({ x, y });

                        float pixelDepth = 0;
                        pixelDepth = glm::dot(bary, { screenSpace.v1.pos.z, screenSpace.v2.pos.z, screenSpace.v3.pos.z });
                       //pixelDepth += bary.x * screenSpace.v1.pos.z;
                       //pixelDepth += bary.y * screenSpace.v2.pos.z;
                       //pixelDepth += bary.z * screenSpace.v3.pos.z;
                        
                        if (pixelDepth > depthBuffer[x + FIRE_W * y])
                        {
                            continue;
                        }

                        //if (pixelDepth > 1)
                        //{
                        //    //std::cout << "Behind near" << std::endl;
                        //    continue;
                        //}

                        depthBuffer[x + FIRE_W * y] = pixelDepth;
                        color = screenSpace.GetPointColor({ x, y });
                        frameBufferOut.put_pixel(x, y, color);
                    }
                }
            }
        }
    }
    
}

void Engine::destroy()
{
    printf("Killing engine \n");
}
