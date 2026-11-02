#include "TerrainGenerator.h"
#include <vector>
#include "Factories/GeometryFactory.h"
#include "Datas/Texture.h"

float TerrainGenerator::SampleHeight(TerrainDesc const& _d, float _x, float _y, glm::vec2 _uv)
{
	glm::vec2 pos = glm::vec2(_x, _y) + _d.noiseOffset;
	
	if (_d.domainWarpStrength > 0.0f)
	{
		glm::vec2 w = {
			PerlinNoise::FractalBrownianMotion(pos.x + 37.1f, pos.y + 11.3f, _d.frequency, 2),
			PerlinNoise::FractalBrownianMotion(pos.x -19.7f, pos.y + 53.9f, _d.frequency, 2), };
		pos += w * _d.domainWarpStrength / _d.frequency;
	}
	
	float n = PerlinNoise::FractalBrownianMotion(pos.x, pos.y, _d.frequency, _d.octaveCount);
	if (_d.ridged)
		n = 1.0f - 2.0f * glm::abs(n);
	
	float h01 = glm::pow(glm::clamp(n * 0.5f + 0.5f, 0.0f, 1.0f), _d.heightRedistribution);
	float h = _d.baseHeight + _d.heightScale * (h01 * 2.0f - 1.0f);
	
	if (_d.useCenterMountain)
	{
		float dst = glm::length(_uv - _d.mountainCenter) / _d.mountainRadius;
		dst += _d.edgeWarp * PerlinNoise::Perlin(pos.x, pos.y, _d.frequency * 0.5f);
		float mask = 1.0f - glm::smoothstep(0.0f, 1.0f, dst);
		h += _d.mountainHeight * mask * (1.0f + _d.mountainNoiseScale * n);
	}
	
	if (_d.terraceSteps > 0)
	{
		float step = (_d.maxHeight - _d.minHeight) / (float)_d.terraceSteps;
		h = glm::round(h / step) * step;
	}
	
	return glm::clamp(h, _d.minHeight, _d.maxHeight);
}

Mesh TerrainGenerator::GetTerrainMesh()
{
	return m_terrain;
}

Vertex MakeVertex(int _x, int _y, Texture const& _heightmap, float _cellSize, uint32_t _sideCellCount)
{
	Vertex v;
	v.position = { (_x - (int)(_sideCellCount / 2)) * _cellSize, _heightmap.Load(_x, _y).r, (_y - (int)(_sideCellCount / 2)) * _cellSize };
	return v;
}

std::vector<Vertex> MakeCell(size_t _index, Texture const& _heightmap, std::vector<glm::vec3> const& _normalmap, float _cellSize, uint32_t _sideCellCount)
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

	uint32_t s = _sideCellCount + 1; // vertices per row
	size_t tl = y * s + x;             // top left
	size_t tr = y * s + (x + 1);       // top right
	size_t bl = (y + 1) * s + x;       // bottom left
	size_t br = (y + 1) * s + (x + 1); // bottom right

	vBuffer[0].normal = _normalmap[tl];
	vBuffer[1].normal = _normalmap[bl];
	vBuffer[2].normal = _normalmap[tr];

	vBuffer[3].normal = _normalmap[bl];
	vBuffer[4].normal = _normalmap[br];
	vBuffer[5].normal = _normalmap[tr];

	return vBuffer;
}

void TerrainGenerator::GenerateMesh(TerrainDesc const& _desc)
{
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

		glm::vec2 uv = glm::vec2(x, y) / (float)(sideVertexCount - 1);
		float value = SampleHeight(_desc, (float)x, (float)y, uv);
		
		heightmap.Store(x, y, vec4(value, value, value, 1.0f));// +2.0f * PerlinNoise::FractalBrownianMotion(x / 4, y / 4, 0.05f, 5));
	}

	// Compute cell normals
	std::vector<glm::vec3> cellNormalMap(m_grid_size * m_grid_size);
	for (size_t i = 0; i < sideVertexCount * sideVertexCount; i++)
	{
		int x = i % sideVertexCount;
		int y = i / sideVertexCount;
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
		normalmap[i] = glm::normalize(normal);
	}
	// Vertice normals
	size_t cellCount = m_grid_size * m_grid_size;

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
