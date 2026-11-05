#ifndef SCENES_TERRAIN_SCENE_H
#define SCENES_TERRAIN_SCENE_H

#include "Scene.h"

#include "../Datas/Lights/LightData.hpp"
#include "../Core/FrameBuffer.hpp"
#include "../Shaders/ShaderBase/Material.h"
#include "../Terrain/TerrainGenerator.h"
#include "../Terrain/Erosion.h"
#include "../Shaders/Shaders.h"

#include "../Cubemap.h"

class Terrain_Scene : public Scene
{
protected:
    void OnInit(Device* _pDevice) override;
    void OnHandleEvent(const SDL_Event& event) override;
    void OnUpdate(float dt) override;
    void OnDraw(Device* _pDevice) override;

    void SetSkybox(Cubemap _cubemap);
    
private:
    void InitLightScene();
    void RegenerateTerrain();
    void PrintTerrainDesc() const;
    void PrintControls() const;
    void ToggleTextures();
    void IncreaseBlending( float _value);
    void IncreaseWaveHeight(float _value);
    void IncreaseWaterLevel(float _value);
    void ToggleWater();

    static constexpr uint32_t TERRAIN_GRID_SIZE = 256;
    static constexpr float TERRAIN_CELL_SIZE = 0.5f;

    Erosion m_eroder; 
    TerrainGenerator m_terrainGenerator;
    TerrainDesc m_terrainDesc;
    TerrainDesc m_defaultTerrainDesc; // restored with numpad 5
    bool m_terrainDirty = false;
    Object* m_terrain = nullptr;
    
    Object* skyboxObject = nullptr;
    Material m_skyboxMat;
    
    Object* m_water = nullptr;
    Material m_waterMaterial;

    bool m_waterToggle = false;
    Water_PS::WaterMat m_waterCb = {};
    WaveSim::SeaParams m_waveCb = { 0.5f, 0.2f, 3.0f, 0.5f };
    float m_targetWaveHeight = 0.2f;
    float m_targetWaterLevel = 0.0f;

    Material m_mat;
    LitTerrain_BP_PS::MaterialCB m_terrainCb;

    PassDesc m_skyboxPass;
    PassDesc m_mainPass;
    PassDesc m_waterPass;

    LightData m_lightData;
};

#endif
