#include "Erosion.h"

#include <glm/exponential.hpp>

// Code based on https://github.com/SebLague/Hydraulic-Erosion/blob/Coding-Adventure-E01/Assets/Scripts/Erosion.cs by Sebastian Lague

void Erosion::Init(GraphicsContext* _ctx)
{
    m_ctx = _ctx;
}

void Erosion::Erode(Mesh const& _mesh, int _mapSize, int _numIterations)
{
    m_mapSize = _mapSize;
    
    m_map = m_ctx->vertexBufferHeap.GetData(_mesh.vertexBuffer);
    
    InitializeBrushIndices(_mapSize, erosionRadius);
    
    srand(seed);
    
    for (int it = 0; it < _numIterations; it++)
    {
        float posX = (float)(rand() % (_mapSize - 1));
        float posY = (float)(rand() % (_mapSize - 1));
        
        float dirX = 0.0f;
        float dirY = 0.0f;
        float speed = initialSpeed;
        float water = initialWaterVolume;
        float sediment = 0.0f;
        
        for (int lifeT = 0; lifeT < maxDropletLifetime; lifeT++)
        {
            int nodeX = (int) posX;
            int nodeY = (int) posY;
            int dropletIndex = nodeY * m_mapSize + nodeX;
            // Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
            float cellOffsetX = posX - (float)nodeX;
            float cellOffsetY = posY - (float)nodeY;
            
            HeightGradient heightGradient = CalcHeightGradient(posX, posY);
            
            dirX = (dirX * inertia - heightGradient.gradientX * (1.0f - inertia));
            dirY = (dirY * inertia - heightGradient.gradientY * (1.0f - inertia));
            
            float len = glm::length(vec2(dirX, dirY));
            if (len != 0.0f)
            {
                dirX /= len;
                dirY /= len;
            }
            
            posX += dirX;
            posY += dirY;
            
            // Stop simulating droplet if it's not moving or has flowed over edge of map
            if (dirX == 0.0f && dirY == 0.0f)
                break;
            if (posX < 0 || posY < 0 || posX >= (float)m_mapSize || posY >= (float)m_mapSize)
                break;
            
            float newHeight = CalcHeightGradient(posX, posY).height;
            float deltaHeight = newHeight - heightGradient.height;
            
            // Calculate the droplet's sediment capacity (higher when moving fast down a slope and contains lots of water)
            float sedimentCapacity = glm::max(-deltaHeight * speed * water * sedimentCapacityFactor, minSedimentCapacity);

            // If carrying more sediment than capacity, or if flowing uphill:
            if (sediment > sedimentCapacity || deltaHeight > 0) 
            {
                // If moving uphill (deltaHeight > 0) try fill up to the current height, otherwise deposit a fraction of the excess sediment
                float amountToDeposit = (deltaHeight > 0) ? glm::min(deltaHeight, sediment) : (sediment - sedimentCapacity) * depositSpeed;
                sediment -= amountToDeposit;

                // Add the sediment to the four nodes of the current cell using bilinear interpolation
                // Deposition is not distributed over a radius (like erosion) so that it can fill small pits
                m_map[dropletIndex].position.y += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
                m_map[dropletIndex + 1].position.y += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
                m_map[dropletIndex + m_mapSize].position.y += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
                m_map[dropletIndex + m_mapSize + 1].position.y += amountToDeposit * cellOffsetX * cellOffsetY;
            }
            else 
            {
                // Erode a fraction of the droplet's current carry capacity.
                // Clamp the erosion to the change in height so that it doesn't dig a hole in the terrain behind the droplet
                float amountToErode = glm::min((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);

                // Use erosion brush to erode from all nodes inside the droplet's erosion radius
                for (size_t brushPointIndex = 0; brushPointIndex < m_erosionBrushIndices[dropletIndex].size(); brushPointIndex++) 
                {
                    int nodeIndex = m_erosionBrushIndices[dropletIndex][brushPointIndex];
                    float weighedErodeAmount = amountToErode * m_erosionBrushWeights[dropletIndex][brushPointIndex];
                    float deltaSediment = (m_map[nodeIndex].position.y < weighedErodeAmount) ? m_map[nodeIndex].position.y : weighedErodeAmount;
                    m_map[nodeIndex].position.y -= deltaSediment;
                    sediment += deltaSediment;
                }
            }

            // Update droplet's speed and water content
            speed = glm::sqrt(speed * speed + deltaHeight * gravity);
            water *= (1 - evaporateSpeed);
        }
    }
}

void Erosion::InitializeBrushIndices(int _mapSize, int _radius)
{
    m_erosionBrushIndices.resize(uint32_t(_mapSize * _mapSize));
    m_erosionBrushWeights.resize(uint32_t(_mapSize * _mapSize));
    
    std::vector<int> xOffsets(uint32_t(_radius * _radius * 4));
    std::vector<int> yOffsets(uint32_t(_radius * _radius * 4));
    std::vector<float> weights(uint32_t(_radius * _radius * 4));
    
    float weightSum = 0.0f;
    int addIndex = 0;
    
    for (size_t i = 0; i < m_erosionBrushIndices.size(); i++)
    {
        int centreX = (int32_t)i % _mapSize;
        int centreY = (int32_t)i / _mapSize;
        
        bool valid = centreY <= _radius || centreY >= _mapSize - _radius || centreX <= _radius + 1 || centreX >= _mapSize - _radius;
        
        if (valid == false)
            continue;
        
        addIndex = 0;
        weightSum = 0.0f;
        for (int y = -_radius; y <= _radius; y++)
        {
            for (int x = -_radius; x <= _radius; x++)
            {
                float sqrDist = (float)(x * x + y * y);
                if (sqrDist >= float(_radius * _radius))
                    continue;
                
                int coordX = centreX + x;
                int coordY = centreY + y;
                
                if (coordX < 0 || coordY < 0)
                    continue;
                if (coordX >= _mapSize || coordY >= _mapSize)
                    continue;
                
                float weight = 1.0f - glm::sqrt(sqrDist) / (float)_radius;
                weightSum += weight;
                weights[addIndex] = weight;
                xOffsets[addIndex] = coordX;
                yOffsets[addIndex] = coordY;
                addIndex++;
            }
        }
        
        int numEntries = addIndex;
        m_erosionBrushIndices[i].resize(numEntries);
        m_erosionBrushWeights[i].resize(numEntries);
        
        for (int j = 0; j < numEntries; j++)
        {
            m_erosionBrushIndices[i][j] = (yOffsets[j] + centreY) * _mapSize + xOffsets[j] + centreX;
            m_erosionBrushWeights[i][j] = weights[j] / weightSum;
        }
    }
}

Erosion::HeightGradient Erosion::CalcHeightGradient(float _posX, float _posY)
{
    int coordX = (int) _posX;
    int coordY = (int) _posY;

    // Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
    float x = _posX - (float)coordX;
    float y = _posY - (float)coordY;

    // Calculate heights of the four nodes of the droplet's cell
    int nodeIndexNW = coordY * m_mapSize + coordX;
    float heightNW = m_map[nodeIndexNW].position.y;
    float heightNE = m_map[nodeIndexNW + 1].position.y;
    float heightSW = m_map[nodeIndexNW + m_mapSize].position.y;
    float heightSE = m_map[nodeIndexNW + m_mapSize + 1].position.y;

    // Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges
    float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
    float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

    // Calculate height with bilinear interpolation of the heights of the nodes of the cell
    float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

    return { height, gradientX, gradientY };
}
