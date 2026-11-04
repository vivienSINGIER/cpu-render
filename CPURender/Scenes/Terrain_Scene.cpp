#include "Terrain_Scene.h"

#include "../Core/Device.h"

#include "../Factories/ShaderFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Datas/Lights/LightHelpers.hpp"

void GenerateTerrainAtlas()
{    
   Handle RockDiff = TextureFactory::Instance()->CreateTexture("RockDiff", "Assets/Textures/Rock/diffuse.jpg");
   Handle RockNorm = TextureFactory::Instance()->CreateTexture("RockNorm", "Assets/Textures/Rock/normal.png");
   Handle RockDisp = TextureFactory::Instance()->CreateTexture("RockDisp", "Assets/Textures/Rock/displacement.png");
   Handle RockRoug = TextureFactory::Instance()->CreateTexture("RockRoug", "Assets/Textures/Rock/roughness.jpg");
   Handle SnowDiff = TextureFactory::Instance()->CreateTexture("SnowDiff", "Assets/Textures/Snow/diffuse.jpg");
   Handle SnowNorm = TextureFactory::Instance()->CreateTexture("SnowNorm", "Assets/Textures/Snow/normal.png");
   Handle SnowDisp = TextureFactory::Instance()->CreateTexture("SnowDisp", "Assets/Textures/Snow/displacement.png");
   Handle SnowRoug = TextureFactory::Instance()->CreateTexture("SnowRoug", "Assets/Textures/Snow/roughness.jpg");

   Handle SandDiff = TextureFactory::Instance()->CreateTexture("SandDiff", "Assets/Textures/Sand/diffuse.jpg");
   Handle SandNorm = TextureFactory::Instance()->CreateTexture("SandNorm", "Assets/Textures/Sand/normal.png");
   Handle SandDisp = TextureFactory::Instance()->CreateTexture("SandDisp", "Assets/Textures/Sand/displacement.png");
   Handle SandRoug = TextureFactory::Instance()->CreateTexture("SandRoug", "Assets/Textures/Sand/roughness.png");
   Handle DrySandDiff = TextureFactory::Instance()->CreateTexture("DrySandDiff", "Assets/Textures/DrySand/diffuse.jpg");;
   Handle DrySandNorm = TextureFactory::Instance()->CreateTexture("DrySandNorm", "Assets/Textures/DrySand/normal.png"); ;
   Handle DrySandDisp = TextureFactory::Instance()->CreateTexture("DrySandDisp", "Assets/Textures/DrySand/displacement.png"); ;
   Handle DrySandRoug = TextureFactory::Instance()->CreateTexture("DrySandRoug", "Assets/Textures/DrySand/roughness.png"); ;

   Handle DirtDiff = TextureFactory::Instance()->CreateTexture("DirtDiff", "Assets/Textures/Dirt/diffuse.jpg");
   Handle DirtNorm = TextureFactory::Instance()->CreateTexture("DirtNorm", "Assets/Textures/Dirt/normal.png");
   Handle DirtDisp = TextureFactory::Instance()->CreateTexture("DirtDisp", "Assets/Textures/Dirt/displacement.png");
   Handle DirtRoug = TextureFactory::Instance()->CreateTexture("DirtRoug", "Assets/Textures/Dirt/roughness.png");
   Handle GrassDiff = TextureFactory::Instance()->CreateTexture("GrassDiff", "Assets/Textures/Grass/diffuse.jpg");
   Handle GrassNorm = TextureFactory::Instance()->CreateTexture("GrassNorm", "Assets/Textures/Grass/normal.png");
   Handle GrassDisp = TextureFactory::Instance()->CreateTexture("GrassDisp", "Assets/Textures/Grass/displacement.png");
   Handle GrassRoug = TextureFactory::Instance()->CreateTexture("GrassRoug", "Assets/Textures/Grass/roughness.png");

   std::vector<Texture*> tex;
   tex.push_back(TextureFactory::Instance()->GetTexture(RockDiff));
   tex.push_back(TextureFactory::Instance()->GetTexture(RockNorm));
   tex.push_back(TextureFactory::Instance()->GetTexture(RockDisp));
   tex.push_back(TextureFactory::Instance()->GetTexture(RockRoug));
   tex.push_back(TextureFactory::Instance()->GetTexture(SnowDiff));
   tex.push_back(TextureFactory::Instance()->GetTexture(SnowNorm));
   tex.push_back(TextureFactory::Instance()->GetTexture(SnowDisp));
   tex.push_back(TextureFactory::Instance()->GetTexture(SnowRoug));

   tex.push_back(TextureFactory::Instance()->GetTexture(SandDiff));
   tex.push_back(TextureFactory::Instance()->GetTexture(SandNorm));
   tex.push_back(TextureFactory::Instance()->GetTexture(SandDisp));
   tex.push_back(TextureFactory::Instance()->GetTexture(SandRoug));
   tex.push_back(TextureFactory::Instance()->GetTexture(DrySandDiff));
   tex.push_back(TextureFactory::Instance()->GetTexture(DrySandNorm));
   tex.push_back(TextureFactory::Instance()->GetTexture(DrySandDisp));
   tex.push_back(TextureFactory::Instance()->GetTexture(DrySandRoug));

   tex.push_back(TextureFactory::Instance()->GetTexture(DirtDiff));
   tex.push_back(TextureFactory::Instance()->GetTexture(DirtNorm));
   tex.push_back(TextureFactory::Instance()->GetTexture(DirtDisp));
   tex.push_back(TextureFactory::Instance()->GetTexture(DirtRoug));
   tex.push_back(TextureFactory::Instance()->GetTexture(GrassDiff));
   tex.push_back(TextureFactory::Instance()->GetTexture(GrassNorm));
   tex.push_back(TextureFactory::Instance()->GetTexture(GrassDisp));
   tex.push_back(TextureFactory::Instance()->GetTexture(GrassRoug));

   Texture::StoreAtlasHDR(tex);
}

void Terrain_Scene::InitLightScene()
{
    m_lightData.lights[0] = MakeDirectionalLight({1.0f, -1.0f, 1.0f}, 2.0f, Color::WHITE);
    m_lightData.ambient = 0.2f;
}

void Terrain_Scene::OnInit(Device* _pDevice)
{
    InitLightScene();
    
    PerlinNoise::InitTable();
    m_eroder.Init(&_pDevice->m_ctx);

    m_mainPass.fb = _pDevice->GetDefaultFrameBuffer();

    m_shadowPass.fb = _pDevice->CreateFrameBuffer(1024, 1024, 0, true);
    m_shadowPass.state.cullMode = CullMode::FRONT;
    
    m_presentTarget = m_mainPass.fb.gBuffer[0];

    _pDevice->SetLightData(m_lightData);

    m_camera.SetShadowDistance(25.0f);
    m_camTransform.SetPosition(glm::vec3(0.0f, 40.0f, -90.f));
    
    m_terrainGenerator.m_grid_size = TERRAIN_GRID_SIZE;
    m_terrainGenerator.m_cellSize = TERRAIN_CELL_SIZE;
    m_terrainDesc.frequency = 0.008f;
    m_terrainDesc.octaveCount = 6;
    m_terrainDesc.heightScale = 7.0f;
    m_terrainDesc.heightRedistribution = 1.7f;
    m_terrainDesc.minHeight = -20.0f;                                                                                                                                                                                                 
    m_terrainDesc.maxHeight = 25.0f;                                                                                                                                                                                                  
    m_terrainDesc.domainWarpStrength = 0.25f;                                                                                                                                                                                         
    m_terrainDesc.useCenterMountain = true;                                                                                                                                                                                           
    m_terrainDesc.mountainCenter = glm::vec2(0.5f);                                                                                                                                                                                   
    m_terrainDesc.mountainRadius = 0.35f;                                                                                                                                                                                             
    m_terrainDesc.mountainHeight = 15.0f;
    m_terrainDesc.edgeWarp = 0.2f;

    m_terrainGenerator.GenerateMesh(m_terrainDesc);
    PrintControls();
    PrintTerrainDesc();
    
    float terrainSide = (m_terrainGenerator.m_grid_size-1) * TERRAIN_CELL_SIZE;

    m_terrainCb = {
        {1.0f, 1.0f, 1.0f, 1.0f},
            Lighting::g_fresnelR0Templates[PLASTIC],
                1.0f, 0.0f, 0.0f, {terrainSide, terrainSide}, true, 0.05f
    };

    Handle Atlas = TextureFactory::Instance()->CreateTexture("TerrainTextureAtlas", "Atlas.hdr");

    m_mat = ShaderFactory::Instance()->CreateMaterial("Lit4T_BP");
    m_mat->SetParams<LitTerrain_BP_PS::MaterialCB>(LitTerrain_BP_PS::CB_SLOT_mat, m_terrainCb);
    m_mat->SetTexture(LitTerrain_BP_VS::TEX_SLOT_texWeight, m_terrainGenerator.m_textureWeights);
    m_mat->SetTexture(LitTerrain_BP_PS::TEX_SLOT_textureAtlas, Atlas);

    m_terrain = CreateObject(STATIC);
    m_terrain->mesh = m_terrainGenerator.GetTerrainMesh();
    m_terrain->mat = m_mat.get();
    m_terrain->transform.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
}

// Terrain controls, physical keys (scancodes) so the numpad works with NumLock on or off. Hold to repeat.
// See PrintControls() for the full list.
void Terrain_Scene::OnHandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
        return;

    TerrainDesc& d = m_terrainDesc;
    bool shift = (event.key.mod & SDL_KMOD_SHIFT) != 0;

    auto add = [](float& _v, float _delta, float _min, float _max) { _v = glm::clamp(_v + _delta, _min, _max); };
    auto mul = [](float& _v, float _factor, float _min, float _max) { _v = glm::clamp(_v * _factor, _min, _max); };

    float offsetStep = 0.25f / d.frequency; // a quarter of a noise feature, whatever the frequency
    float centerStep = 0.02f;               // mountain center, in 0..1 grid space
    float heightLimitStep = 0.5f;

    switch (event.key.scancode)
    {
    case SDL_SCANCODE_RETURN: m_eroder.Erode(m_terrain->mesh, d, TERRAIN_GRID_SIZE, 50'000); return;

    case SDL_SCANCODE_T: ToggleTextures(); return;
    case SDL_SCANCODE_B: IncreaseBlending(+0.001f); return;
    case SDL_SCANCODE_N: IncreaseBlending(-0.001f); return;
        
    // --- Arrows : noise offset | Shift : mountain center ---
    case SDL_SCANCODE_LEFT:  if (shift) add(d.mountainCenter.x, -centerStep, 0.0f, 1.0f); else d.noiseOffset.x -= offsetStep; break;
    case SDL_SCANCODE_RIGHT: if (shift) add(d.mountainCenter.x,  centerStep, 0.0f, 1.0f); else d.noiseOffset.x += offsetStep; break;
    case SDL_SCANCODE_UP:    if (shift) add(d.mountainCenter.y,  centerStep, 0.0f, 1.0f); else d.noiseOffset.y += offsetStep; break;
    case SDL_SCANCODE_DOWN:  if (shift) add(d.mountainCenter.y, -centerStep, 0.0f, 1.0f); else d.noiseOffset.y -= offsetStep; break;

    // --- Numpad : noise and height ---
    case SDL_SCANCODE_KP_7: mul(d.frequency, 1.0f / 1.1f, 0.001f, 1.0f); break;
    case SDL_SCANCODE_KP_9: mul(d.frequency, 1.1f, 0.001f, 1.0f); break;

    case SDL_SCANCODE_KP_4: d.octaveCount = glm::max(d.octaveCount, 2u) - 1; break;
    case SDL_SCANCODE_KP_6: d.octaveCount = glm::min(d.octaveCount + 1, 16u); break;

    case SDL_SCANCODE_KP_1: add(d.heightScale, -0.25f, 0.0f, 100.0f); break;
    case SDL_SCANCODE_KP_3: add(d.heightScale,  0.25f, 0.0f, 100.0f); break;

    case SDL_SCANCODE_KP_2: add(d.baseHeight, -0.25f, -50.0f, 50.0f); break;
    case SDL_SCANCODE_KP_8: add(d.baseHeight,  0.25f, -50.0f, 50.0f); break;

    case SDL_SCANCODE_KP_MINUS: mul(d.heightRedistribution, 1.0f / 1.1f, 0.1f, 8.0f); break;
    case SDL_SCANCODE_KP_PLUS:  mul(d.heightRedistribution, 1.1f, 0.1f, 8.0f); break;

    case SDL_SCANCODE_KP_DIVIDE:   add(d.domainWarpStrength, -0.05f, 0.0f, 3.0f); break;
    case SDL_SCANCODE_KP_MULTIPLY: add(d.domainWarpStrength,  0.05f, 0.0f, 3.0f); break;

    case SDL_SCANCODE_KP_0:      d.ridged = !d.ridged; break;
    case SDL_SCANCODE_KP_PERIOD: d.terraceSteps = (d.terraceSteps + 1) % 21; break; // 0 (off) .. 20, wraps
    case SDL_SCANCODE_KP_ENTER:  d.useCenterMountain = !d.useCenterMountain; break;
    case SDL_SCANCODE_KP_5:      d = m_defaultTerrainDesc; break;

    // --- Ins/Del/Home/End/PgUp/PgDn : mountain | Shift : edge warp & height limits ---
    case SDL_SCANCODE_HOME:     if (shift) add(d.maxHeight,  heightLimitStep, d.minHeight + heightLimitStep, 100.0f); else add(d.mountainHeight,  0.5f, 0.0f, 100.0f); break;
    case SDL_SCANCODE_END:      if (shift) add(d.maxHeight, -heightLimitStep, d.minHeight + heightLimitStep, 100.0f); else add(d.mountainHeight, -0.5f, 0.0f, 100.0f); break;
    case SDL_SCANCODE_PAGEUP:   if (shift) add(d.minHeight,  heightLimitStep, -100.0f, d.maxHeight - heightLimitStep); else add(d.mountainRadius,  0.02f, 0.02f, 2.0f); break;
    case SDL_SCANCODE_PAGEDOWN: if (shift) add(d.minHeight, -heightLimitStep, -100.0f, d.maxHeight - heightLimitStep); else add(d.mountainRadius, -0.02f, 0.02f, 2.0f); break;
    case SDL_SCANCODE_INSERT:   if (shift) add(d.edgeWarp,  0.05f, 0.0f, 2.0f); else add(d.mountainNoiseScale,  0.05f, 0.0f, 2.0f); break;
    case SDL_SCANCODE_DELETE:   if (shift) add(d.edgeWarp, -0.05f, 0.0f, 2.0f); else add(d.mountainNoiseScale, -0.05f, 0.0f, 2.0f); break;

    default: return;
    }

    m_terrainDirty = true;
}

void Terrain_Scene::OnUpdate(float dt)
{
    // Regenerate at most once per frame, even if several keys were pressed
    if (m_terrainDirty)
    {
        m_terrainDirty = false;
        RegenerateTerrain();
    }
}

void Terrain_Scene::RegenerateTerrain()
{
    // Free the previous mesh buffers, the heaps are not blocked outside of the frame
    m_pDevice->m_ctx.vertexBufferHeap.Delete(m_terrain->mesh.vertexBuffer);
    m_pDevice->m_ctx.indexBufferHeap.Delete(m_terrain->mesh.indexBuffer);

    m_terrainGenerator.GenerateMesh(m_terrainDesc);
    m_terrain->mesh = m_terrainGenerator.GetTerrainMesh();
    MarkStaticDirty(); // recompute world bounds + octree for the new mesh

    PrintTerrainDesc();
}

void Terrain_Scene::PrintTerrainDesc() const
{
    TerrainDesc const& d = m_terrainDesc;
    std::cout << "--- Terrain ---\n"
              << "  noise    : offset (" << d.noiseOffset.x << ", " << d.noiseOffset.y << ") | frequency " << d.frequency
              << " | octaves " << d.octaveCount << " | warp " << d.domainWarpStrength << " | ridged " << (d.ridged ? "on" : "off") << "\n"
              << "  height   : scale " << d.heightScale << " | base " << d.baseHeight << " | redistribution " << d.heightRedistribution
              << " | min " << d.minHeight << " | max " << d.maxHeight << " | terraces " << d.terraceSteps << "\n"
              << "  mountain : " << (d.useCenterMountain ? "on" : "off") << " | center (" << d.mountainCenter.x << ", " << d.mountainCenter.y << ")"
              << " | height " << d.mountainHeight << " | radius " << d.mountainRadius << " | noise " << d.mountainNoiseScale
              << " | edge warp " << d.edgeWarp << "\n";
}

void Terrain_Scene::PrintControls() const
{
    std::cout << "=== Terrain controls ===\n"
              << "  Arrows            : noise offset X / Y      Shift + Arrows      : mountain center\n"
              << "  Num 7 / 9         : frequency - / +         Num 4 / 6           : octaves - / +\n"
              << "  Num 1 / 3         : height scale - / +      Num 2 / 8           : base height - / +\n"
              << "  Num - / +         : redistribution - / +    Num / / *           : domain warp - / +\n"
              << "  Num 0             : ridged on/off           Num .               : terrace steps (0..20, wraps)\n"
              << "  Num Enter         : mountain on/off         Num 5               : reset all\n"
              << "  Home / End        : mountain height + / -   Shift + Home / End  : max height + / -\n"
              << "  PgUp / PgDn       : mountain radius + / -   Shift + PgUp / PgDn : min height + / -\n"
              << "  Ins / Del         : mountain noise + / -    Shift + Ins / Del   : edge warp + / -\n"
              << "=== Render controls ===\n"
              << "  T                 : toggle textures         B / N               : terrain blending + / -\n";
}

void Terrain_Scene::ToggleTextures()
{
    m_terrainCb.useTextures = !m_terrainCb.useTextures;
    m_mat->SetParams<LitTerrain_BP_PS::MaterialCB>(LitTerrain_BP_PS::CB_SLOT_mat, m_terrainCb);
}

void Terrain_Scene::IncreaseBlending(float _value)
{
    m_terrainCb.blendDistance += _value;
    m_terrainCb.blendDistance = clamp(m_terrainCb.blendDistance, 0.0f, 0.5f);
    m_mat->SetParams<LitTerrain_BP_PS::MaterialCB>(LitTerrain_BP_PS::CB_SLOT_mat, m_terrainCb);
}

void Terrain_Scene::OnDraw(Device* _pDevice)
{
    Frustum shadowFrustum;
    glm::mat4x4 lightSpace = Lighting::ComputeDirLightViewProj(m_shadowPass.passData.view, m_camera.GetShadowProj(),
                                        m_lightData.lights[0].light.direction, m_shadowPass.fb.width, shadowFrustum, 0.0f);

    m_camera.FillData(&m_mainPass.passData);
    m_mainPass.passData.deltaTime = m_deltaTime;
    m_mainPass.passData.totalTime = m_totalTime;
    m_mainPass.passData.lightSpace = lightSpace;

    m_camera.FillData(&m_shadowPass.passData);
    m_shadowPass.passData.viewProj = lightSpace;

    // _pDevice->BeginPass(m_shadowPass);
    //     DrawObjects(_pDevice, shadowFrustum);
    // _pDevice->EndPass();

    _pDevice->BeginPass(m_mainPass);
        DrawObjects(_pDevice, m_camera.GetFrustum());
    _pDevice->EndPass();
}
