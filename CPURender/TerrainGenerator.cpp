#include "TerrainGenerator.h"
#include <vector>
#include "Factories/GeometryFactory.h"
#include "Datas/Texture.h"

Mesh TerrainGenerator::GetTerrainMesh()
{
	return m_terrain;
}

Vertex MakeVertex(int _x, int _y, Texture _heightmap, float _cellSize, uint32_t _sideCellCount)
{
	Vertex v;
	v.position = { (_x - (int)(_sideCellCount / 2)) * _cellSize, _heightmap.GetPixelData<float>(_x, _y), (_y - (int)(_sideCellCount / 2)) * _cellSize };
	return v;
}

std::vector<Vertex> MakeCell(size_t _index, Texture _heightmap, float _cellSize, uint32_t _sideCellCount)
{
	std::vector<Vertex> vBuffer;

	int x = _index % _sideCellCount;
	int y = _index / _sideCellCount;

	vBuffer.push_back(MakeVertex(x, y, _heightmap, _cellSize, _sideCellCount));
	vBuffer.push_back(MakeVertex(x, y+1, _heightmap, _cellSize, _sideCellCount));
	vBuffer.push_back(MakeVertex(x+1, y, _heightmap, _cellSize, _sideCellCount));
	
	glm::vec3 normalA = glm::cross(vBuffer[1].position - vBuffer[0].position, vBuffer[2].position - vBuffer[0].position);
	glm::vec3 normalB = glm::cross(vBuffer[4].position - vBuffer[3].position, vBuffer[5].position - vBuffer[3].position);

	glm::dot(normalA, normalB);

	vBuffer[0].normal = normalA;
	vBuffer[1].normal = normalA;
	vBuffer[2].normal = normalA;

	vBuffer.push_back(MakeVertex(x, y+1, _heightmap, _cellSize, _sideCellCount));
	vBuffer.push_back(MakeVertex(x+1, y+1, _heightmap, _cellSize, _sideCellCount));
	vBuffer.push_back(MakeVertex(x+1, y, _heightmap, _cellSize, _sideCellCount));

	vBuffer[3].normal = normalB;
	vBuffer[4].normal = normalB;
	vBuffer[5].normal = normalB;



	return vBuffer;
}

void TerrainGenerator::GenerateMesh()
{
	PerlinNoise::InitTable();
	
	uint32_t sideVertexCount = m_grid_size + 1; 

	std::vector<Vertex> gridV;
	std::vector<Index> gridI;

	// Fill Heightmap
	size_t pointCount = sideVertexCount * sideVertexCount;
	Texture heightmap = Texture(sideVertexCount, sideVertexCount, Texture::MORTON);
	Texture texMap = Texture(sideVertexCount, sideVertexCount, Texture::MORTON);
	for (size_t i = 0; i < pointCount; i++)
	{
		int x = i % sideVertexCount;
		int y = i / sideVertexCount;

		heightmap.SetPixelData<float>(x, y, 0.8f * PerlinNoise::FractalBrownianMotion(x, y, 0.01f, 8) + 1.0f * PerlinNoise::FractalBrownianMotion(x / 2, y / 2, 0.05f, 5));
	}

	// Fill Vertex and Index buffer
	size_t cellCount = m_grid_size * m_grid_size;
	for (size_t i = 0; i < cellCount; i++)
	{
		std::vector<Vertex> subVBuff = MakeCell(i, heightmap, m_cellSize, m_grid_size);
		gridV.insert(gridV.end(), subVBuff.begin(), subVBuff.end());

		gridI.push_back(i * 6); gridI.push_back(i * 6 + 1); gridI.push_back(i * 6 + 2);
		gridI.push_back(i * 6 + 3); gridI.push_back(i * 6 + 4); gridI.push_back(i * 6 + 5);
	}

	m_terrain = GeometryFactory::Instance()->CreateMesh(gridV, gridI);
}
