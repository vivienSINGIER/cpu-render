#ifndef CORE_BINNING_HPP_DEFINED
#define CORE_BINNING_HPP_DEFINED

#include <array>
#include <vector>

#include "../main.h"
#include "../Datas/Mesh.h"
#include "../Datas/Triangle.h"

#define TILE_SIZE 32

#define TILE_COUNT_W ((WIDTH + TILE_SIZE - 1) / TILE_SIZE)
#define TILE_COUNT_H ((HEIGHT + TILE_SIZE - 1) / TILE_SIZE)
#define TILE_COUNT (TILE_COUNT_W * TILE_COUNT_H)

namespace Binning
{
    struct Tile
    {
        std::vector<Index> indices;
        int x = 0, y = 0;
    };
    
    using Tiles = std::array<Tile, TILE_COUNT>;
    
    struct BoundingBox
    {
        int minX, minY, maxX, maxY;
    };
    
    inline BoundingBox GetBoundingBox(Triangle _tri)
    {
        float minX = _tri.v1.positionH.x;
        float maxX = _tri.v1.positionH.x;
        float minY = _tri.v1.positionH.y;
        float maxY = _tri.v1.positionH.y;

        minX = _tri.v2.positionH.x < minX ? _tri.v2.positionH.x : minX;
        minX = _tri.v3.positionH.x < minX ? _tri.v3.positionH.x : minX;

        maxX = _tri.v2.positionH.x > maxX ? _tri.v2.positionH.x : maxX;
        maxX = _tri.v3.positionH.x > maxX ? _tri.v3.positionH.x : maxX;

        minY = _tri.v2.positionH.y < minY ? _tri.v2.positionH.y : minY;
        minY = _tri.v3.positionH.y < minY ? _tri.v3.positionH.y : minY;

        maxY = _tri.v2.positionH.y > maxY ? _tri.v2.positionH.y : maxY;
        maxY = _tri.v3.positionH.y > maxY ? _tri.v3.positionH.y : maxY;
        
        return BoundingBox{(int)floor(minX), (int)floor(minY), (int)ceil(maxX), (int)ceil(maxY)};
    }
    
    inline void InitTiles(Tiles& _out)
    {
#pragma omp parallel
#pragma omp for
        for (int i = 0; i < TILE_COUNT; i++)
        {
            int y = i / TILE_COUNT_W;
            int x = i % TILE_COUNT_W;
            _out[i].x = x * TILE_SIZE;
            _out[i].y = y * TILE_SIZE;
        }
    }
    
    bool TrivialReject(Triangle const& _tri, Tile const& _tile)
    {
        
    }
    
    inline void WriteOverlappedTiles(Triangle const& _tri, Index _index, Tiles& _out)
    {
        BoundingBox box = GetBoundingBox(_tri);
        box.minX = std::max(box.minX, 0);
        box.minY = std::max(box.minY, 0);
        box.maxX = std::min(box.maxX, WIDTH);
        box.maxY = std::min(box.maxY, HEIGHT);
        
        if (box.minX == box.maxX)
            return;
        if (box.minY == box.maxY)
            return;
        
        int tileMinX = box.minX / TILE_SIZE;
        int tileMaxX = box.maxX / TILE_SIZE;
        int tileMinY = box.minY / TILE_SIZE;
        int tileMaxY = box.maxY / TILE_SIZE;
        
        for (int i = tileMinX; i < tileMaxX; i++)
        {
            for (int j = tileMinY; j < tileMaxY; j++)
            {
                Tile& t = _out[i + TILE_COUNT_W * j];
                t.indices.push_back(_index);
            }
        }
    }
    
}

#endif
