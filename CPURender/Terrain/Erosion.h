#ifndef TERRAIN_ERROSION_HPP_DEFINED
#define TERRAIN_ERROSION_HPP_DEFINED
#include <vector>

#include "../Core/GraphicsContext.h"
#include "TerrainGenerator.h"

class Erosion
{
public:
    int seed = 1;
    
    int erosionRadius = 3;
    float inertia = .05f; // At zero, water will instantly change direction to flow downhill. At 1, water will never change direction. 
    float sedimentCapacityFactor = 4; // Multiplier for how much sediment a droplet can carry
    float minSedimentCapacity = .01f; // Used to prevent carry capacity getting too close to zero on flatter terrain
    
    float erodeSpeed = .3f;
    float depositSpeed = .3f;
    
    float evaporateSpeed = .01f;
    float gravity = 4;
    int maxDropletLifetime = 30;
    
    float initialWaterVolume = 1;
    float initialSpeed = 1;
    
    void Init(GraphicsContext* _ctx);
    
    // Heights are normalized from [baseHeight - heightScale, baseHeight + heightScale] to [0, 1] for the
    // simulation, so the erosion parameters behave the same whatever the terrain's vertical scale.
    void Erode(Mesh const& _mesh, TerrainDesc const& _desc, int _mapSize, int _numIterations = 1);
    
private:
    
    struct HeightGradient
    {
        float height;
        float gradientX;
        float gradientY;
    };
    
    Vertex* m_map;
    int m_mapSize;

    std::vector<float> m_heights; // normalized heights the simulation works on
    float m_heightMin = 0.0f;     // world height mapped to 0
    float m_heightRange = 1.0f;   // world height span mapped to [0, 1]
    
    void InitializeBrushIndices(int _mapSize, int _radius);
    HeightGradient CalcHeightGradient(float _posX, float _posY);
    void RecomputeNormals();
    
    std::vector<std::vector<int>> m_erosionBrushIndices;
    std::vector<std::vector<float>> m_erosionBrushWeights;
    
    GraphicsContext* m_ctx;
};

#endif