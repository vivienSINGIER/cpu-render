#include "TerrainGenerator.h"
#include <vector>
#include "Factories/GeometryFactory.h"

Mesh TerrainGenerator::GetTerrainMesh()
{
	return m_terrain;
}

void TerrainGenerator::GenerateMesh()
{
	PerlinNoise::InitTable();

	uint32_t sideVertexCount = m_grid_size + 1; 

	std::vector<Vertex> gridV;
	std::vector<Index> gridI;

	// Fill Vertex buffer
	size_t vertexCount = sideVertexCount * sideVertexCount;
	for (size_t i = 0; i < vertexCount; i++)
	{
		Vertex current;
		int x = i % sideVertexCount;
		int y = i / sideVertexCount;

		float height = PerlinNoise::FractalBrownianMotion(x, y, 0.7f, 3);

		current.position = { (x - (int)(sideVertexCount / 2)) * m_cellSize, (y - (int)(sideVertexCount / 2)) * m_cellSize,  height };
		// Doesn't need UV; Textures are world aligned

		gridV.push_back(current);
	}
	
	// Fill Index buffer
	size_t cellCount = m_grid_size * m_grid_size;
	for (size_t i = 0; i < cellCount; i++)
	{
		Index topLeft = i;
		Index topRight = i + 1;
		Index bottomLeft = sideVertexCount + i;
		Index bottomRight = sideVertexCount + i + 1;

		gridI.push_back(bottomLeft); gridI.push_back(topRight); gridI.push_back(topLeft);		// Top left triangle
		gridI.push_back(bottomLeft); gridI.push_back(bottomRight); gridI.push_back(topRight);	// Bottom right triangle
	}

	m_terrain = GeometryFactory::Instance()->CreateMesh(gridV, gridI);
}
