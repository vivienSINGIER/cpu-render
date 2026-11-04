#pragma once
#include "../Datas/Mesh.h"
#include "PerlinNoise.h"

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

struct TerrainDesc
{
	float frequency = 0.1f;
	uint32_t octaveCount = 8;

	glm::vec2 noiseOffset = glm::vec2(0.0f);
	
	float heightScale = 1.0f;
	float baseHeight = 0.0f;
	float heightRedistribution = 1.0f;
	float minHeight = -10.0f;
	float maxHeight = 10.0f;
	
	bool useCenterMountain = false;
	glm::vec2 mountainCenter = glm::vec2(0.0f);
	float mountainHeight = 8.0f;
	float mountainRadius = 1.0f;
	float mountainNoiseScale = 0.5f;
	float edgeWarp = 0.5f;
	
	bool ridged = false;
	int terraceSteps = 0;
	float domainWarpStrength = 0.0f;
};

class TerrainGenerator
{
	// Align 3 noises:   
	// 1 for Water/ground height
	// 1 for "biomes" / texture type
	// 1 for small scale heightmap
	Mesh m_terrain;

	float SampleHeight(TerrainDesc const& _d, float _x, float _y, glm::vec2 _uv);
	glm::vec4 SampleTex(TerrainDesc const& _d, float _x, float _y, float _height);
	
public:
	// Square grid
	float m_cellSize; // Length of a side
	uint32_t m_grid_size; // m_grid_size * m_grid_size => vertexCount

	Handle m_textureWeights;

	Mesh GetTerrainMesh();
	void GenerateMesh(TerrainDesc const& _terrainDesc);
};

