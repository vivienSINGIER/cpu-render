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
	v.position = { (_x - (int)(_sideCellCount / 2)) * _cellSize, _heightmap.Load(_x, _y).r, (_y - (int)(_sideCellCount / 2)) * _cellSize };
	return v;
}

std::vector<Vertex> MakeCell(size_t _index, Texture _heightmap, std::vector<glm::vec3> _normalmap, float _cellSize, uint32_t _sideCellCount)
{
	std::vector<Vertex> vBuffer;

	int x = _index % _sideCellCount;
	int y = _index / _sideCellCount;

	vBuffer.push_back(MakeVertex(x, y, _heightmap, _cellSize, _sideCellCount)); // top left
	vBuffer.push_back(MakeVertex(x, y+1, _heightmap, _cellSize, _sideCellCount)); // bottom left
	vBuffer.push_back(MakeVertex(x+1, y, _heightmap, _cellSize, _sideCellCount)); // top right

	vBuffer.push_back(MakeVertex(x, y + 1, _heightmap, _cellSize, _sideCellCount)); // bottom left
	vBuffer.push_back(MakeVertex(x + 1, y + 1, _heightmap, _cellSize, _sideCellCount)); // bottom right
	vBuffer.push_back(MakeVertex(x + 1, y, _heightmap, _cellSize, _sideCellCount)); // top right

	vBuffer[0].normal = _normalmap[_index];
	vBuffer[1].normal = _normalmap[_index + _sideCellCount];
	vBuffer[2].normal = _normalmap[_index + 1];

	vBuffer[3].normal = _normalmap[_index + _sideCellCount];
	vBuffer[4].normal = _normalmap[_index + _sideCellCount + 1];
	vBuffer[5].normal = _normalmap[_index + 1];

	/*vBuffer[0].normal = _normalmap[_index];
	vBuffer[1].normal = _normalmap[_index];
	vBuffer[2].normal = _normalmap[_index];

	vBuffer[3].normal = _normalmap[_index];
	vBuffer[4].normal = _normalmap[_index];
	vBuffer[5].normal = _normalmap[_index];*/

	return vBuffer;
}

void TerrainGenerator::GenerateMesh()
{
	PerlinNoise::InitTable();
	
	uint32_t sideVertexCount = m_grid_size + 1; 

	std::vector<Vertex> gridV;
	std::vector<Index> gridI;

	std::vector<glm::vec3> normalmap(sideVertexCount * sideVertexCount);
	Texture texMap = Texture(sideVertexCount, sideVertexCount,  Texture::R32F, Texture::MORTON);

	// Fill Heightmap
	size_t pointCount = sideVertexCount * sideVertexCount;
	Texture heightmap = Texture(sideVertexCount, sideVertexCount, Texture::R32F, Texture::MORTON);
	for (size_t i = 0; i < pointCount; i++)
	{
		int x = i % sideVertexCount;
		int y = i / sideVertexCount;

		float value = 1.f*PerlinNoise::FractalBrownianMotion(x, y, 0.01f, 8);
		
		heightmap.Store(x, y, vec4(value, value, value, 1.0f));// +2.0f * PerlinNoise::FractalBrownianMotion(x / 4, y / 4, 0.05f, 5));
	}

	// Compute cell normals
	std::vector<glm::vec3> cellNormalMap(m_grid_size * m_grid_size);
	for (size_t i = 0; i < m_grid_size * m_grid_size; i++)
	{
		int x = i % m_grid_size;
		int y = i / m_grid_size;
		glm::vec3 pos = { x * m_cellSize, heightmap.Load(x, y).r, y * m_cellSize };

		glm::vec3 normal = glm::vec3(0, 1, 0);

		glm::vec3 right = glm::vec3(0, 0, 0);
		glm::vec3 left = glm::vec3(0, 0, 0);

		glm::vec3 down = glm::vec3(0, 0, 0);
		glm::vec3 up = glm::vec3(0, 0, 0);

		glm::vec3 tu = {};
		glm::vec3 tv = {};

		if (x > 0 && x < sideVertexCount - 1)
		{
			glm::vec3 left = { (x - 1) * m_cellSize, heightmap.Load(x - 1, y).r, y * m_cellSize };
			glm::vec3 right = { (x + 1) * m_cellSize, heightmap.Load(x + 1, y).r, y * m_cellSize };
			tu = right - left;
		}
		else if (x == 0)
		{
			glm::vec3 right = { (x + 1) * m_cellSize, heightmap.Load(x + 1, y).r, y * m_cellSize };
			tu = right - pos;
		}
		else
		{
			glm::vec3 left = { (x - 1) * m_cellSize, heightmap.Load(x - 1, y).r, y * m_cellSize };
			tu = pos - left;
		}
	
		if (y > 0 && y < sideVertexCount - 1)
		{
			glm::vec3 top = { x * m_cellSize, heightmap.Load(x, y - 1).r, (y - 1) * m_cellSize };
			glm::vec3 bottom = { x * m_cellSize, heightmap.Load(x, y + 1).r, (y + 1) * m_cellSize };
			tv = top - bottom;
		}
		else if (y == 0)
		{
			glm::vec3 bottom = { x * m_cellSize, heightmap.Load(x, y + 1).r, (y + 1) * m_cellSize };
			tv = pos - bottom;
		}
		else
		{
			glm::vec3 top = { x * m_cellSize, heightmap.Load(x, y - 1).r, (y - 1) * m_cellSize };
			tv = top - pos;
		}
	
		normal = glm::cross(tu, tv);
		cellNormalMap[i] = glm::normalize(normal);
	}
	// Vertice normals
	size_t cellCount = m_grid_size * m_grid_size;
	for (size_t i = 0; i < cellCount; i++)
	{
		int x = i % m_grid_size;
		int y = i / m_grid_size;
		glm::vec3 normal = glm::vec3(0, 0, 0);

		if (x > 0 && y > 0) // Get top left
		{
			normal += cellNormalMap[i - 1 - m_grid_size];
		}; 

		if (x < m_grid_size - 1 && y > 0) // Get top right
		{
			normal += cellNormalMap[i + 1 - m_grid_size];
		};

		if (x > 0 && y < m_grid_size - 1) // Get bottom left
		{
			normal += cellNormalMap[i - 1 + m_grid_size];
		}

		if (x < m_grid_size - 1 && y < m_grid_size - 1) // Get bottom right
		{
			normal += cellNormalMap[i + 1 + m_grid_size];
		}

		normalmap[i] = glm::normalize(normal);
	}

	// Fill Vertex and Index buffer
	for (size_t i = 0; i < cellCount; i++)
	{
		std::vector<Vertex> subVBuff = MakeCell(i, heightmap, normalmap, m_cellSize, m_grid_size);
		gridV.insert(gridV.end(), subVBuff.begin(), subVBuff.end());

		gridI.push_back(i * 6); gridI.push_back(i * 6 + 1); gridI.push_back(i * 6 + 2);
		gridI.push_back(i * 6 + 3); gridI.push_back(i * 6 + 4); gridI.push_back(i * 6 + 5);
	}

	m_terrain = GeometryFactory::Instance()->CreateMesh(gridV, gridI);
}
