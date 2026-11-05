#include "TerrainGenerator.h"
#include <vector>
#include "../Factories/GeometryFactory.h"
#include "../Factories/TextureFactory.h"
#include "../Datas/Texture.h"

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

glm::vec4 TerrainGenerator::SampleTex(TerrainDesc const& _d, float _x, float _y, float _height)
{
	uint32_t sideVertexCount = m_grid_size;
	glm::vec2 uv = glm::vec2(_x, _y) / (float)(sideVertexCount - 1);

	float height = (_height - _d.minHeight) / (_d.maxHeight - _d.minHeight);
	height += PerlinNoise::FractalBrownianMotion(_x * 2.0f -4.1f, _y * 1.8f + 12.0f, 0.02f, 4) * 0.05f; // Add noise
	glm::vec4 weights = glm::vec4(height, 0.0f, 0.0f, 0.0f);

	return weights;
}

Mesh TerrainGenerator::GetTerrainMesh()
{
	return m_terrain;
}

void TerrainGenerator::GenerateMesh(TerrainDesc const& _desc)
{
	uint32_t sideVertexCount = m_grid_size; 

	std::vector<Vertex> gridV;
	std::vector<Index> gridI;
	
	Texture texMap = Texture(sideVertexCount, sideVertexCount,  Texture::R32F, Texture::MORTON);

	// Fill Heightmap and Texture Weight Texture
	size_t vertexCount = sideVertexCount * sideVertexCount;
	gridV.resize(vertexCount);
	Texture heightmap = Texture(sideVertexCount, sideVertexCount, Texture::R32F, Texture::MORTON);
	if(m_textureWeights.IsValid() == false) m_textureWeights = TextureFactory::Instance()->CreateTexture("terrainTextureWeight", m_grid_size, m_grid_size);
	Texture* texWeight = TextureFactory::Instance()->GetTexture(m_textureWeights);
	for (size_t i = 0; i < vertexCount; i++)
	{
		int x = i % sideVertexCount;
		int y = i / sideVertexCount;

		glm::vec2 uv = glm::vec2(x, y) / (float)(sideVertexCount - 1);
		float value = SampleHeight(_desc, (float)x, (float)y, uv);
		
		assert(value == value);
		
		heightmap.Store(x, y, vec4(value, value, value, 1.0f));
		
		glm::vec4 weight = SampleTex(_desc, x, y, heightmap.GetPixelData<glm::vec4>(x, y).x);
		texWeight->SetPixelData<glm::vec4>(x, y, weight);

		vec3 pos = { ((float)x - (float)((int32)m_grid_size / 2)) * m_cellSize, value, ((float)y - (float)((int32)m_grid_size / 2)) * m_cellSize };
		
		gridV[i] = { pos, uv };
	}

	// Compute cell normals
	// Clamped fetch so edge vertices use a one-sided difference
	auto P = [&](int x, int y) -> vec3
		{
			x = glm::clamp(x, 0, (int)sideVertexCount - 1);
			y = glm::clamp(y, 0, (int)sideVertexCount - 1);
			return gridV[y * sideVertexCount + x].position;
		};

	for (int y = 0; y < sideVertexCount; y++)
	{
		for (int x = 0; x < sideVertexCount; x++)
		{
			vec3 dx = P(x + 1, y) - P(x - 1, y);   // tangent along columns
			vec3 dz = P(x, y + 1) - P(x, y - 1);   // tangent along rows

			gridV[y * sideVertexCount + x].normal = glm::normalize(glm::cross(dz, dx));
		}
	}
	
	uint32_t cellPerRow = m_grid_size - 1;
	size_t cellCount = (m_grid_size - 1) * (m_grid_size - 1);
	// Fill Index buffer
	for (size_t i = 0; i < cellCount; i++)
	{
		int x = (int32)(i % cellPerRow);
		int y = (int32)(i / cellPerRow);
		
		uint32_t s = sideVertexCount; // vertices per row
		Index tl = y * s + x;             // top left
		Index tr = y * s + (x + 1);       // top right
		Index bl = (y + 1) * s + x;       // bottom left
		Index br = (y + 1) * s + (x + 1); // bottom right
		
		gridI.push_back(tl); 
		gridI.push_back(bl); 
		gridI.push_back(tr);
		gridI.push_back(bl); 
		gridI.push_back(br); 
		gridI.push_back(tr);
	}

	// Create Terrain Mesh
	m_terrain = GeometryFactory::Instance()->CreateMesh(gridV, gridI);
}
