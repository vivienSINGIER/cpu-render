#include "engine.h"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <map>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define NEAR 0.1f
#define FAR 100.0f

enum SideMask { NONE = 0b0000, LEFT = 0b0001, RIGHT = 0b0010, UP = 0b0100, DOWN = 0b1000};

bool Intersecting(float& t, float &u, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
    t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
    u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
    return (t <= 1 && t >= 0 && u <= 1 && u >= 0);
}

std::pair<std::shared_ptr<Vertex_OUT>, SideMask> FindIntersectionWithVP(Vertex_OUT v1, Vertex_OUT v2, glm::vec4 viewport)
{
    //v1:
    float x1 = glm::vec2(v1.pos).x;
    float y1 = glm::vec2(v1.pos).y;

    // v2:
    float x2 = glm::vec2(v2.pos).x;
    float y2 = glm::vec2(v2.pos).y;

    glm::vec4 left = { viewport.x, viewport.y, viewport.x, viewport.w };
    glm::vec4 up = { viewport.x, viewport.w, viewport.z, viewport.w };
    glm::vec4 right = { viewport.z, viewport.y, viewport.z, viewport.w };
    glm::vec4 down = { viewport.x, viewport.y, viewport.z, viewport.y };

    SideMask intersectingSide = NONE;

    float t, u;
    if (x1 < viewport.x) // Left
    {
        float x3 = left.x;
        float y3 = left.y;
        float x4 = left.z;
        float y4 = left.w;

        if (Intersecting(t, u, x1, y1, x2, y2, x3, y3, x4, y4)) intersectingSide = LEFT;
    }
    if (x1 > viewport.z && intersectingSide == NONE) // Right
    {
        float x3 = right.x;
        float y3 = right.y;
        float x4 = right.z;
        float y4 = right.w;

        if (Intersecting(t, u, x1, y1, x2, y2, x3, y3, x4, y4)) intersectingSide = RIGHT;
    }
    if (y1 < viewport.y && intersectingSide == NONE) // Down
    {
        float x3 = down.x;
        float y3 = down.y;
        float x4 = down.z;
        float y4 = down.w;

        if (Intersecting(t, u, x1, y1, x2, y2, x3, y3, x4, y4)) intersectingSide = DOWN;
    }
    if (y1 > viewport.w && intersectingSide == NONE) // Up
    {
        float x3 = up.x;
        float y3 = up.y;
        float x4 = up.z;
        float y4 = up.w;

        if (Intersecting(t, u, x1, y1, x2, y2, x3, y3, x4, y4)) intersectingSide = UP;
    }

    if (intersectingSide == NONE) return { nullptr, NONE};

    std::shared_ptr<Vertex_OUT> intersection = std::make_shared<Vertex_OUT>();
    intersection->pos = { x1 + (x2 - x1) * t, y1 + (y2 - y1) * t, 0 };
    intersection->pos.z = v1.pos.z * (1 - t) + v2.pos.z * t;
    intersection->color = v1.color * (1 - t) + v2.color * t;


    return { intersection, intersectingSide };
}

bool CheckAddVPCorner(glm::vec2 _pos, Triangle _tri, SideMask _first, SideMask _second, std::vector<std::shared_ptr<Vertex_OUT>>& _vertices, std::map<std::shared_ptr<Vertex_OUT>, int>& _sideMap)
{
    // If true, find two adjacent sides closest new vertex (or next corner if no vertex) and slot in between
    // or slot after the previous draw order side (see clockwise/counterclockwise)

    if (IsInTriangle(_pos, _tri) == false) return false;
    {
        std::shared_ptr<Vertex_OUT> intersection = std::make_shared<Vertex_OUT>();
        intersection = std::make_shared<Vertex_OUT>(InterpolateVertex(_pos, _tri));
        size_t vSize = _vertices.size();
        int lastSide = NONE;
        std::shared_ptr<Vertex_OUT> lastSideVert = nullptr;
        bool foundBoth = false;
        for (int i = 0; i < vSize; i++)
        {
            if (lastSide == NONE && (_sideMap[_vertices[i]] & _second || _sideMap[_vertices[i]] & _first)) { lastSide = _sideMap[_vertices[i]]; }
            if (_sideMap[_vertices[i]] & lastSide)
            {
                lastSideVert = _vertices[i];
            }
            //insert
            if (lastSide & _first && _sideMap[_vertices[i]] & _second)
            {
                auto it = std::find(_vertices.begin(), _vertices.end(), _vertices[i]);
                _vertices.insert(it, intersection);
                foundBoth = true;
                break;
            }
            if ((lastSide & _second && _sideMap[_vertices[i]] & _first))
            {
                auto it = std::find(_vertices.begin(), _vertices.end(), _vertices[i]);
                _vertices.insert(it + 1, intersection);
                foundBoth = true;
                break;
            }
        }

        if (foundBoth == false)
        {
            // Case in which no intersection and no vertice in vp
            if (lastSide == NONE) { 
                _vertices.push_back(intersection);
                //std::cout << "Error: could not find a common side for corner" << std::endl; return false; 
            }

            else if (lastSide & _first)
            {
                auto it = std::find(_vertices.begin(), _vertices.end(), lastSideVert);
                _vertices.insert(it+1, intersection);
            }
            else if (lastSide & _second)
            {
                auto it = std::find(_vertices.begin(), _vertices.end(), lastSideVert);
                _vertices.insert(it, intersection);
            }
        }

        _sideMap[intersection] = _first | _second;
        return true;
    }
}

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
    debugTriangle->transform.Scale(1.0f);

    debugTriangle2->mesh = triangleMesh;
    debugTriangle2->transform.SetPosition(glm::vec3(0.5f, 0, 2.5f));

    objects.push_back(debugTriangle);
    //objects.push_back(debugTriangle2);
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN)

    

    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_SPACE)
        {
            camDirection |= 0b000001;
            //mainCam.Translate({ 0, 0.5f, 0 });
        }
        if (event.key.key == SDLK_LCTRL)
        {
            camDirection |= 0b000010;
            //mainCam.Translate({ 0, -0.5f, 0 });
        }
        if (event.key.key == SDLK_Z)
        {
            camDirection |= 0b000100;
            //mainCam.Translate({ 0, 0, 0.5f});
        }
        if (event.key.key == SDLK_S)
        {
            camDirection |= 0b001000;
            //mainCam.Translate({ 0, 0, -0.5f });
        }
        if (event.key.key == SDLK_Q)
        {
            camDirection |= 0b010000;
            //mainCam.Translate({ 0.5f, 0, 0 });
        }
        if (event.key.key == SDLK_D)
        {
            camDirection |= 0b100000;
            //mainCam.Translate({ -0.5f, 0, 0 });
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
    if (event.type == SDL_EVENT_KEY_UP)
    {
        if (event.key.key == SDLK_SPACE)
        {
            camDirection &= !0b000001;
            //mainCam.Translate({ 0, 0.5f, 0 });
        }
        if (event.key.key == SDLK_LCTRL)
        {
            camDirection &= !0b000010;
            //mainCam.Translate({ 0, -0.5f, 0 });
        }
        if (event.key.key == SDLK_Z)
        {
            camDirection &= !0b000100;
            //mainCam.Translate({ 0, 0, 0.5f});
        }
        if (event.key.key == SDLK_S)
        {
            camDirection &= !0b001000;
            //mainCam.Translate({ 0, 0, -0.5f });
        }
        if (event.key.key == SDLK_Q)
        {
            camDirection &= !0b010000;
            //mainCam.Translate({ 0.5f, 0, 0 });
        }
        if (event.key.key == SDLK_D)
        {
            camDirection &= !0b100000;
            //mainCam.Translate({ -0.5f, 0, 0 });
        }
    }

}

void Engine::renderFrame(Texture& _tex)
{
    // DEBUG: REMOVE LATER
    {
        const float spd = 0.1f;
        if(camDirection & 0b000001) mainCam.Translate({ 0, spd, 0 });
        if(camDirection & 0b000010) mainCam.Translate({ 0, -spd, 0 });
        if(camDirection & 0b000100) mainCam.Translate({ 0, 0, spd });
        if(camDirection & 0b001000) mainCam.Translate({ 0, 0, -spd });
        if(camDirection & 0b010000) mainCam.Translate({ spd, 0, 0 });
        if(camDirection & 0b100000) mainCam.Translate({ -spd, 0, 0 });
    }

    _tex.Clear(Color::BLACK);

    //objects[0]->transform.AddYPR(glm::vec3(0, (float)target_frame, 0)); //::rotate(objects[0]->transform, 3.1415f * 4.0f / (float)target_frame, { 0, 1, 0 });

    // Create Depth Buffer
    size_t pixelCount = _tex.Size();
    std::vector<float> depthBuffer = std::vector<float>(pixelCount);
    std::fill(depthBuffer.begin(), depthBuffer.end(), FAR);

    glm::mat4x4 proj = glm::perspectiveFov(M_PI/3.f, (float)WIDTH, (float)HEIGHT, NEAR, FAR);

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

            Triangle currentTriangle = { std::make_shared<Vertex_OUT>(v1), std::make_shared<Vertex_OUT>(v2), std::make_shared<Vertex_OUT>(v3) };
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
    size_t currentIndex = 0;
    for (int j = 0; j < vCount; j += 3)
    {
                      // 321
        int clipMask = 0b000;
        Triangle currentTriangle = { std::make_shared<Vertex_OUT>(vBuffer[currentIndex]), std::make_shared<Vertex_OUT>(vBuffer[currentIndex + 1]), std::make_shared<Vertex_OUT>(vBuffer[currentIndex + 2]) };
        clipMask = (IsInVP(currentTriangle.v1->pos) == false) | ((IsInVP(currentTriangle.v2->pos) == false) << 1) | ((IsInVP(currentTriangle.v3->pos) == false) << 2);

        if (clipMask == 0b000)
        {
            currentIndex += 3;
            continue;
        }

        std::map<std::shared_ptr<Vertex_OUT>, int> sideMap;

        std::shared_ptr<Vertex_OUT> v[3] = { currentTriangle.v1 , currentTriangle.v2 , currentTriangle.v3 };

        std::vector<std::shared_ptr<Vertex_OUT>> clippedVertices; // remove at end if out of vp
        clippedVertices.push_back(currentTriangle.v1); sideMap[currentTriangle.v1] = NONE;
        clippedVertices.push_back(currentTriangle.v2); sideMap[currentTriangle.v2] = NONE;
        clippedVertices.push_back(currentTriangle.v3); sideMap[currentTriangle.v3] = NONE;

        // Intersection
        for (int i = 0; i < 3; i++)
        {
            if (((clipMask >> i) & 1) == 1)
            {
                // 2 possible new vertex, one on v2 - v1 other on v3 - v1
                // v1->v2
                std::pair<std::shared_ptr<Vertex_OUT>, SideMask> intersectionDataA = FindIntersectionWithVP(*v[i], *v[(i + 1) % 3], viewport);
                if (intersectionDataA.first != nullptr)
                {
                    auto it = std::find(clippedVertices.begin(), clippedVertices.end(), v[i]);
                    clippedVertices.insert(it + 1, intersectionDataA.first);
                    sideMap[intersectionDataA.first] = intersectionDataA.second;
                }
                // v1->v3
                std::pair<std::shared_ptr<Vertex_OUT>, SideMask> intersectionDataB = FindIntersectionWithVP(*v[i], *v[(i + 2) % 3], viewport);
                if (intersectionDataB.first != nullptr)
                {
                    auto it = std::find(clippedVertices.begin(), clippedVertices.end(), v[i]);
                    clippedVertices.insert(it, intersectionDataB.first);
                    sideMap[intersectionDataB.first] = intersectionDataB.second;
                }
            }
        }

        //Corners:
        CheckAddVPCorner({ viewport.x, viewport.w }, currentTriangle, LEFT, UP, clippedVertices, sideMap);
        CheckAddVPCorner({ viewport.z, viewport.w }, currentTriangle, UP, RIGHT, clippedVertices, sideMap);
        CheckAddVPCorner({ viewport.z, viewport.y }, currentTriangle, RIGHT, DOWN, clippedVertices, sideMap);
        CheckAddVPCorner({ viewport.x, viewport.y }, currentTriangle, DOWN, LEFT, clippedVertices, sideMap);
        
        // Remove initial oob vertice
        if(((clipMask >> 0) & 1) == 1) clippedVertices.erase(std::find(clippedVertices.begin(), clippedVertices.end(), currentTriangle.v1));
        if(((clipMask >> 1) & 1) == 1) clippedVertices.erase(std::find(clippedVertices.begin(), clippedVertices.end(), currentTriangle.v2));
        if(((clipMask >> 2) & 1) == 1) clippedVertices.erase(std::find(clippedVertices.begin(), clippedVertices.end(), currentTriangle.v3));
        
        std::vector<std::shared_ptr<Vertex_OUT>> ordered;
        int newVertCount = clippedVertices.size();
        if(newVertCount >= 3)
        {
            for (int i = 0; i < newVertCount - 2; i++)
            {
                ordered.push_back(clippedVertices[0]);
                ordered.push_back(clippedVertices[i + 1]);
                ordered.push_back(clippedVertices[i + 2]);
            }
        }
        
        vBuffer.erase(vBuffer.begin() + currentIndex, vBuffer.begin() + currentIndex + 3); // Remove initial triangle
        auto it = vBuffer.begin() + currentIndex;
        size_t orderedSize = ordered.size();
        for (int i = 0; i < orderedSize; i++)
        {
            it = vBuffer.insert(it, *(ordered[i]));// Add new triangles
        }
        currentIndex += orderedSize;
    }

    vCount = vBuffer.size();
    //////// Raster ////////
    for (int j = 0; j < vCount; j += 3)
    {
        Triangle currentTriangle = { std::make_shared<Vertex_OUT>(vBuffer[j]), std::make_shared<Vertex_OUT>(vBuffer[j + 1]), std::make_shared<Vertex_OUT>(vBuffer[j + 2]) };
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
                    Vertex_OUT px = InterpolateVertex({ x, y }, currentTriangle);
                    float pixelDepth = px.pos.z;
                        
                    //////// PS ////////

                    if (pixelDepth > depthBuffer[x + WIDTH * y]) continue;
                    if (pixelDepth > 1) continue;

                    depthBuffer[x + WIDTH * y] = pixelDepth;

                    _tex.SetPixelColor(x, HEIGHT - y - 1, px.color);
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
    return viewport.x <= _pos.x && viewport.z >= _pos.x &&
        viewport.y <= _pos.y && viewport.w >= _pos.y;
}

void Engine::destroy()
{
    printf("Killing engine \n");
}
