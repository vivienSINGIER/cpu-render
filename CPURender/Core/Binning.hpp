#ifndef CORE_BINNING_HPP_DEFINED
#define CORE_BINNING_HPP_DEFINED

#include <vector>

#include "../Datas/Mesh.h"
#include "../Datas/Triangle.h"

#define TILE_SIZE 32

namespace Binning
{
    struct BoundingBox
    {
        int minX, minY, maxX, maxY;
    };
    
    struct BinEntry
    {
        Index index;
        BoundingBox box;
        glm::vec3 barOffsetX;
        glm::vec3 barOffsetY;
    };
    
    struct Tile
    {
        std::vector<BinEntry> entries;
        int x = 0, y = 0;
    };
    
    struct Tiles
    {
        std::vector<Tile> tiles;
        int width = 0, height = 0;
        int countW = 0, countH = 0;

        int Count() const { return countW * countH; }
        Tile&       operator[](int _i)       { return tiles[_i]; }
        Tile const& operator[](int _i) const { return tiles[_i]; }
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
    
    inline void InitTiles(Tiles& _out, int _width, int _height)
    {
        if (_out.width == _width && _out.height == _height)
            return;

        _out.width = _width;
        _out.height = _height;
        _out.countW = (_width + TILE_SIZE - 1) / TILE_SIZE;
        _out.countH = (_height + TILE_SIZE - 1) / TILE_SIZE;
        _out.tiles.assign(_out.Count(), Tile{});

        for (int i = 0; i < _out.Count(); i++)
        {
            int y = i / _out.countW;
            int x = i % _out.countW;
            _out[i].x = x * TILE_SIZE;
            _out[i].y = y * TILE_SIZE;
        }
    }
    
    inline void ClearTiles(Tiles& _inOut)
    {
        for (Tile& t : _inOut.tiles)
            t.entries.clear();
    }
    
    inline bool TrivialReject(Triangle const& _tri, Tile const& _tile)
    {
        glm::vec3 states[4];
        int factorsX[4] = {0, 0, 1, 1};
        int factorsY[4] = {0, 1, 0, 1};
        
        for (int i = 0; i < 4; i++)
        {
            glm::vec2 point = { _tile.x + factorsX[i] * TILE_SIZE , _tile.y + factorsY[i] * TILE_SIZE };
            glm::vec3 bary = BarycentricWeight(point, _tri);
            
            states[i] = {bary.x, bary.y, bary.z};
        }
        
        for (int i = 0; i < 3; i++)
        {
            bool isCorrect = false;
            for (int j = 0; j < 4; j++)
                isCorrect = isCorrect || states[j][i] >= 0;
            
            if (!isCorrect)
                return true;
        }
        
        return false;
    }
    
    // TODO add TrivialAccept
    
    inline void WriteOverlappedTiles(Triangle const& _tri, Index _index, Tiles& _out)
    {
        BoundingBox box = Binning::GetBoundingBox(_tri);
        box.minX = std::max(box.minX, 0);
        box.minY = std::max(box.minY, 0);
        box.maxX = std::min(box.maxX, _out.width);
        box.maxY = std::min(box.maxY, _out.height);
        
        if (box.minX == box.maxX)
            return;
        if (box.minY == box.maxY)
            return;
        
        int tileMinX = box.minX / TILE_SIZE;
        int tileMinY = box.minY / TILE_SIZE;
        int tileMaxX = (box.maxX - 1) / TILE_SIZE;
        int tileMaxY = (box.maxY - 1) / TILE_SIZE;
        
        glm::vec3 barOffsetX;
        glm::vec3 barOffsetY;
        BarycentricOffsets(_tri, barOffsetX, barOffsetY);
        
        for (int i = tileMinX; i <= tileMaxX; i++)
        {
            for (int j = tileMinY; j <= tileMaxY; j++)
            {
                Tile& t = _out[i + _out.countW * j];
                if (TrivialReject(_tri, t))
                    continue;
                
                BoundingBox newB;
                newB.minX = std::max(box.minX, t.x);
                newB.minY = std::max(box.minY, t.y);
                newB.maxX = std::min(box.maxX, t.x + TILE_SIZE);
                newB.maxY = std::min(box.maxY, t.y + TILE_SIZE);
                
                t.entries.push_back({_index, newB, barOffsetX, barOffsetY});
            }
        }
    }
    
    inline void BlinningStep(std::vector<Vertex_OUT> const& _vBuffer, Tiles& _out)
    {
        for (uint32_t v = 0; v < _vBuffer.size(); v += 3)
        {
            Triangle currentTriangle = { _vBuffer[v], _vBuffer[v + 1], _vBuffer[v + 2] };
            WriteOverlappedTiles(currentTriangle, v, _out);
        }
    }
    
}

#endif
