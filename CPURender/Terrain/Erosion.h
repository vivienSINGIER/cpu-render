#ifndef TERRAIN_ERROSION_HPP_DEFINED
#define TERRAIN_ERROSION_HPP_DEFINED
#include <vector>

#include "../Core/GraphicsContext.h"

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
    
    void Erode(Mesh const& _mesh, int _mapSize, int _numIterations = 1);
    
private:
    
    struct HeightGradient
    {
        float height;
        float gradientX;
        float gradientY;
    };
    
    Vertex* m_map;
    int m_mapSize;
    
    void InitializeBrushIndices(int _mapSize, int _radius);
    HeightGradient CalcHeightGradient(float _posX, float _posY);
    
    std::vector<std::vector<int>> m_erosionBrushIndices;
    std::vector<std::vector<float>> m_erosionBrushWeights;
    
    GraphicsContext* m_ctx;
};

#endif