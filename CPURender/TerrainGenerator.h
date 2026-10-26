#pragma once
#include "Datas/Mesh.h"
#include "Maths/PerlinNoise.h"

enum TerrainType
{
	DIRT,
	SAND,
	ROCK,
	GRASS
};

struct TerrainData
{
	TerrainType m_type;
	float m_height;
};

class TerrainGenerator
{
	// Align 3 noises: 
	// 1 for Water/ground height
	// 1 for "biomes" / texture type
	// 1 for small scale heightmap
	Mesh m_terrain;

public:
	// Square grid
	float m_cellSize; // Length of a side
	uint32_t m_grid_size; // m_grid_size * m_grid_size => cellCount

	Mesh GetTerrainMesh();
	void GenerateMesh();
};

