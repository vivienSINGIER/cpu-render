#include "engine.h"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define NEAR 0.1f
#define FAR 100.0f


void Engine::init()
{
    mainCam = Camera();
    mainCam.transform = glm::identity<glm::mat4x4>();

    viewport = glm::vec4(0, 0, WIDTH, HEIGHT);

    std::shared_ptr<Object> debugTriangle = std::make_shared<Object>();
    std::shared_ptr<Object> debugTriangle2 = std::make_shared<Object>();
    std::shared_ptr<Object> debugTriangle3 = std::make_shared<Object>();

    Mesh triangleMesh;
    triangleMesh.vertice.push_back({ glm::vec3(1.0f, 0.0f, 0.0f), Color::BLUE });
    triangleMesh.vertice.push_back({ glm::vec3(0.0f, 1.0f, 0.0f), Color::GREEN });
    triangleMesh.vertice.push_back({ glm::vec3(-1.0f, 0.0f, 0.0f), Color::RED });

    debugTriangle->mesh = triangleMesh;
    debugTriangle->transform.SetPosition(glm::vec3(0, 0, 2));

    debugTriangle2->mesh = triangleMesh;
    debugTriangle2->transform.SetPosition(glm::vec3(0.5f, 0, 2.5f));

    objects.push_back(debugTriangle);
    objects.push_back(debugTriangle2);
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        if (event.key.key == SDLK_SPACE)
        {
            mainCam.Translate({ 0, 0.5f, 0 });
        }
        if (event.key.key == SDLK_LCTRL)
        {
            mainCam.Translate({ 0, -0.5f, 0 });
        }
        if (event.key.key == SDLK_S)
        {
            mainCam.Translate({ 0, 0, -0.5f });
        }
        if (event.key.key == SDLK_Z)
        {
            mainCam.Translate({ 0, 0, 0.5f});
        }
        if (event.key.key == SDLK_Q)
        {
            mainCam.Translate({ 0.5f, 0, 0 });
        }
        if (event.key.key == SDLK_D)
        {
            mainCam.Translate({ -0.5f, 0, 0 });
        }
        if (event.key.key == SDLK_A)
        {
            mainCam.RotateYaw(3.1415f / 16.0f);
        }
        if (event.key.key == SDLK_E)
        {
            mainCam.RotateYaw(-3.1415f / 16.0f);
        }
    }
}

void Engine::renderFrame(Texture& _tex)
{
    _tex.Clear(Color::BLACK);

    //objects[0]->transform.AddYPR(glm::vec3(0, (float)target_frame, 0)); //::rotate(objects[0]->transform, 3.1415f * 4.0f / (float)target_frame, { 0, 1, 0 });

    // Create Depth Buffer
    size_t pixelCount = _tex.Size();
    std::vector<float> depthBuffer = std::vector<float>(pixelCount);
    std::fill(depthBuffer.begin(), depthBuffer.end(), FAR);

    glm::mat4x4 proj = glm::perspectiveFov(M_PI/2.f, (float)WIDTH, (float)HEIGHT, NEAR, FAR);

    //glm::mat4x4 view = glm::inverse(mainCam.transform);
    glm::mat4x4 view = glm::lookAt(glm::vec3(mainCam.transform[3]), glm::vec3(mainCam.transform[2]) + glm::vec3(mainCam.transform[3]), {0, 1, 0});
    std::vector<Vertex_OUT> vBuffer;

    size_t objCount = objects.size();
    // VERTEX //
    for (int i = 0; i < objCount; i++)
    {
        glm::mat4x4 modelView = view * objects[i]->transform.GetMatrix();

        size_t meshVCount = objects[i]->mesh.vertice.size();
        if (meshVCount % 3 != 0)
        {
            std::cout << "Vertex count not divisible by 3!" << std::endl;
            continue;
        }

        for (int j = 0; j < meshVCount; j += 3)
        {
            Vertex_OUT v1 = VS(objects[i]->mesh.vertice[j], modelView, proj);
            Vertex_OUT v2 = VS(objects[i]->mesh.vertice[j + 1], modelView, proj);
            Vertex_OUT v3 = VS(objects[i]->mesh.vertice[j + 2], modelView, proj);

            Triangle currentTriangle = { v1, v2, v3 };
            if (IsFront(currentTriangle) == false) {
                continue; // Discard 
            }

            vBuffer.push_back(v1);
            vBuffer.push_back(v2);
            vBuffer.push_back(v3);
        }
    }

    size_t vCount = vBuffer.size();
    if (vCount % 3 != 0)
    {
        std::cout << "Vertex count not divisible by 3! (something went wrong)" << std::endl;
        return;
    }
    //////// Clipping //////// (TODO) TO REFACTOR
    //for (int j = 0; j < vCount; j += 3)
    //{
    //    int currentIndex = 0;

    //                  // 321
    //    int clipMask = 0b000;
    //    Triangle currentTriangle = { vBuffer[currentIndex], vBuffer[currentIndex + 1], vBuffer[currentIndex + 2] };
    //    clipMask = IsInVP(currentTriangle.v1.pos) | (IsInVP(currentTriangle.v2.pos) << 1) | (IsInVP(currentTriangle.v3.pos) << 2);

    //    if (clipMask == 0b000)
    //    {
    //        currentIndex += 3;
    //        continue;
    //    }

    //    int sideMask = 0b0000; // LEFT UP RIGHT DOWN
    //    std::vector<int> sideMasks;

    //    glm::vec4 left = { viewport.x, viewport.y, viewport.x, viewport.w };
    //    glm::vec4 up = { viewport.x, viewport.y, viewport.z, viewport.y };
    //    glm::vec4 right = { viewport.z, viewport.y, viewport.z, viewport.w };
    //    glm::vec4 down = { viewport.x, viewport.w, viewport.z, viewport.w };

    //    // New vertex always placed either on previous line, or in vp corner => They are ALWAYS used

    //    Vertex_OUT v[3] = { currentTriangle.v1 , currentTriangle.v2 , currentTriangle.v3 };

    //    std::vector<Vertex_OUT> clippedVertices; // remove at end if out of vp
    //    clippedVertices.push_back(currentTriangle.v1);
    //    clippedVertices.push_back(currentTriangle.v2);
    //    clippedVertices.push_back(currentTriangle.v3);

    //    // Intersection
    //    for (int i = 0; i < 3; i++)
    //    {
    //        if (((clipMask << i) & 1) == 1)
    //        {
    //            // 2 new vertex, one on v2 - v1 other on v3 - v1
    //            
    //            // v1v2:
    //            glm::vec4 dir = { glm::vec2(v[i].pos), glm::vec2(v[(i + 1)%3].pos) };

    //            Vertex_OUT intersection1;

    //            //...
    //            float distanceFactor = 0; // todo
    //            intersection1.pos;
    //            //intersection1.color = v[i].color * (1-distanceFactor) + v[(i + 1) % 3].color * distanceFactor; //interpolate data


    //            // v1v3
    //            dir = { glm::vec2(v[i].pos), glm::vec2(v[(i + 2)%3].pos) };

    //            Vertex_OUT intersection2;

    //            //...
    //            distanceFactor = 0; // todo
    //            intersection2.pos;
    //            //intersection2.color = v[i].color * (1-distanceFactor) + v[(i + 1) % 3].color * distanceFactor; //interpolate data

    //            auto it = std::find(clippedVertices.begin(), clippedVertices.end(), v[i]);

    //            clippedVertices.insert(it+1, intersection1); // after
    //            clippedVertices.insert(it, intersection2); // before
    //        }
    //    }
    //    

    //    //Corner:
    //    //Is corner in triangle?
    //    if (IsInTriangle({ viewport.x, viewport.y }, currentTriangle))
    //    {

    //    }
    //    //If true, find two adjacent sides closest new vertex (or next corner if no vertex) and slot in between
    //    // or slot after the previous draw order side (see clockwise/counterclockwise)

    //    // Remove initial oob vertice
    //    if(((clipMask << 0) & 1) == 1) clippedVertices.erase(std::find(clippedVertices.begin(), clippedVertices.end(), currentTriangle.v1));
    //    if(((clipMask << 1) & 1) == 1) clippedVertices.erase(std::find(clippedVertices.begin(), clippedVertices.end(), currentTriangle.v2));
    //    if(((clipMask << 2) & 1) == 1) clippedVertices.erase(std::find(clippedVertices.begin(), clippedVertices.end(), currentTriangle.v3));

    //    std::vector<Vertex_OUT> ordered;
    //    for (int i = 0; i < clippedVertices.size() - 2; i++)
    //    {
    //        ordered.push_back(clippedVertices[0]);
    //        ordered.push_back(clippedVertices[i+1]);
    //        ordered.push_back(clippedVertices[i+2]);
    //    }

    //    vBuffer.erase(vBuffer.begin() + currentIndex, vBuffer.begin() + currentIndex + 3); // Remove initial triangle
    //    vBuffer.insert(vBuffer.begin() + currentIndex, ordered.begin(), ordered.end()); // Add new triangles
    //    currentIndex += ordered.size();
    //}

    //////// Raster ////////
    for (int j = 0; j < vCount; j += 3)
    {
        Triangle currentTriangle = { vBuffer[j], vBuffer[j + 1], vBuffer[j + 2] };
        glm::vec4 box = GetBoundingBox(currentTriangle);
        box.x = fmax(box.x, 0);
        box.y = fmax(box.y, 0);
        box.z = fmin(box.z, WIDTH);
        box.w = fmin(box.w, HEIGHT);
    
        for (int32_t x = box.x; x < box.z; x++) {
            for (int32_t y = box.y; y < box.w; y++) {
                Color color;
                if (IsInTriangle({ x, y }, currentTriangle))
                {
                    glm::vec3 bary = BarycentricWeight({ x, y }, currentTriangle);
                    float pixelDepth = 0;
                    pixelDepth = glm::dot(bary, { currentTriangle.v1.pos.z, currentTriangle.v2.pos.z, currentTriangle.v3.pos.z });
                        
                    //////// PS ////////

                    if (pixelDepth > depthBuffer[x + WIDTH * y]) continue;
                    if (pixelDepth > 1) continue;

                    depthBuffer[x + WIDTH * y] = pixelDepth;

                    color = GetPointColor({ x, y }, currentTriangle);
                    _tex.SetPixelColor(x, HEIGHT - y - 1, color);
                }
            }
        }
    }
    
}

Vertex_OUT Engine::VS(Vertex_IN _v, glm::mat4x4 _modelView, glm::mat4x4 _proj)
{
    Vertex_OUT vOut;
    vOut.color = _v.color;
    vOut.pos = glm::project(_v.pos, _modelView, _proj, viewport);
    return vOut;
}

bool Engine::IsInVP(glm::vec2 _pos)
{
    return viewport.x >= _pos.x && viewport.z <= _pos.x &&
        viewport.y >= _pos.y && viewport.w <= _pos.y;
}

void Engine::destroy()
{
    printf("Killing engine \n");
}
