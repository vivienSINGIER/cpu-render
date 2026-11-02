#ifndef SCENES_TERRAIN_SCENE_HPP
#define SCENES_TERRAIN_SCENE_HPP

#include "Scene.h"

#include "../Datas/Lights/LightData.hpp"
#include "../Core/FrameBuffer.hpp"
#include "../Shaders/ShaderBase/Material.h"
#include "../Terrain/TerrainGenerator.h"
#include "../Terrain/Erosion.h"

class Terrain_Scene : public Scene
{
protected:
    void OnInit(Device* _pDevice) override;
    void OnHandleEvent(const SDL_Event& event) override;
    void OnUpdate(float dt) override;
    void OnDraw(Device* _pDevice) override;

private:
    void InitLightScene();
    void RegenerateTerrain();
    void PrintTerrainDesc() const;
    void PrintControls() const;

    static constexpr uint32_t TERRAIN_GRID_SIZE = 512;
    static constexpr float TERRAIN_CELL_SIZE = 0.25f;

    Erosion m_eroder; 
    TerrainGenerator m_terrainGenerator;
    TerrainDesc m_terrainDesc;
    TerrainDesc m_defaultTerrainDesc; // restored with numpad 5
    bool m_terrainDirty = false;
    Object* m_terrain = nullptr;

    Material m_mat;

    PassDesc m_shadowPass;
    PassDesc m_mainPass;

    LightData m_lightData;
};

#endif
