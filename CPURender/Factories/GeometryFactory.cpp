#include "GeometryFactory.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <unordered_map>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

#include "../Core/GraphicsContext.h"

void GeometryFactory::Init(GraphicsContext* _pContext)
{
	assert(g_instance == nullptr && "GeometryFactory already initialized");
	g_instance = new GeometryFactory(_pContext);
}

GeometryFactory* GeometryFactory::Instance()
{
	assert(g_instance != nullptr && "GeometryFactory must be initialized by a GraphicsContext");
	return g_instance;
}

GeometryFactory::GeometryFactory(GraphicsContext* _pContext) : m_ctx(_pContext)
{
}

Mesh GeometryFactory::CreateMesh(std::vector<Vertex>& _vertices, std::vector<Index>& _indices)
{
	Mesh m;
	m.vertexCount = (uint32_t)_vertices.size();
	m.indexCount = (uint32_t)_indices.size();
	m.vertexBuffer = m_ctx->vertexBufferHeap.Create(_vertices.data(), (uint16_t)m.vertexCount);
	m.indexBuffer = m_ctx->indexBufferHeap.Create(_indices.data(), (uint16_t)m.indexCount);

	glm::vec3& box_min = m.local_box.min;
	glm::vec3& box_max = m.local_box.max;
	box_min = _vertices[0].position;
	box_max = _vertices[0].position;

	for (int i = 1; i < _vertices.size(); ++i)
	{
		box_min = glm::min(box_min, _vertices[i].position);
		box_max = glm::max(box_max, _vertices[i].position);
	}

	return m;
}

#pragma region BUILDERS

////////////////////////////////////////////
//// PYRAMID
////////////////////////////////////////////

Mesh GeometryFactory::BuildPyramid(bool _randomColors)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;


	vertices = {
		// Front face
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},

		// Right Face
		Vertex{glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3( 0.5f, -0.5f, 00.5f), glm::vec2(1.0f, 1.0f)},

		// Back Face
		Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3( -0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 1.0f)},

		// Left Face
		Vertex{glm::vec3( -0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3( -0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},

		Vertex{glm::vec3( 0.0f,  0.5f,  0.0f), glm::vec2(0.5f, 0.0f)},

		// Bottom Face
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3( -0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 1.0f)}
	};
	indices = {
		9, 10, 11,
		9, 11, 12,

		0, 8, 1, // Front Face
		2, 8, 3, // Right Face
		4, 8, 5, // Back Face
		6, 8, 7, // Left Face
	};

	vertices = CalculateNormalsAndTangents(vertices, indices);
	if (_randomColors)
		ApplyPaletteColors(vertices);

	return CreateMesh(vertices, indices);
}

////////////////////////////////////////////
//// CUBE
////////////////////////////////////////////

Mesh GeometryFactory::BuildCube(bool _randomColors)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	vertices = {
		// DOWN  (y = -0.5)
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
		// UP    (y = +0.5)
		Vertex{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 1.0f)},
		// LEFT  (x = -0.5)
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
		// RIGHT (x = +0.5)
		Vertex{glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
		// FRONT (z = -0.5)
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
		// BACK  (z = +0.5)
		Vertex{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
	};

	indices = {
		// DOWN
		0,  2,  1,   3,  1,  2,
	   // UP
		4,  5,  6,   7,  6,  5,
	   // LEFT
		8,  9, 10,   9, 11, 10,
	   // RIGHT
	   12, 14, 13,  13, 14, 15,
	   // FRONT
	   16, 18, 17,  17, 18, 19,
	   // BACK
	   20, 21, 22,  21, 23, 22,
   };

	vertices = CalculateNormalsAndTangents(vertices, indices);
	if (_randomColors)
		ApplyPaletteColors(vertices);

	return CreateMesh(vertices, indices);
}

Mesh GeometryFactory::BuildSkybox()
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	const float s = 1.0f/3.0f;
	const glm::vec2 dFactor = glm::vec2(1*s, 2*s);
	const glm::vec2 uFactor = glm::vec2(0*s, 2*s);
	const glm::vec2 lFactor = glm::vec2(0*s, 1*s);
	const glm::vec2 rFactor = glm::vec2(1*s, 1*s);
	const glm::vec2 fFactor = glm::vec2(0*s, 0*s);
	const glm::vec2 bFactor = glm::vec2(1*s, 0*s);

	vertices = {
		// DOWN  (y = -0.5)
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) * s + dFactor},
		Vertex{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f) * s + dFactor},
		Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f) * s + dFactor},
		Vertex{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f) * s + dFactor},
		// UP    (y = +0.5)
		Vertex{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f) * s + uFactor},
		Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 1.0f) * s + uFactor},
		Vertex{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 0.0f) * s + uFactor},
		Vertex{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 1.0f) * s + uFactor},
		// LEFT  (x = -0.5)
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)*s + lFactor},
		Vertex{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 1.0f)*s + lFactor},
		Vertex{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f)*s + lFactor},
		Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)*s + lFactor},
		// RIGHT (x = +0.5)
		Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f)*s + rFactor},
		Vertex{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 1.0f)*s + rFactor},
		Vertex{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 0.0f)*s + rFactor},
		Vertex{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f)*s + rFactor},
		// FRONT (z = -0.5)
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f) * s + bFactor},
		Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) * s + bFactor},
		Vertex{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 0.0f) * s + bFactor},
		Vertex{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f) * s + bFactor},
		// BACK  (z = +0.5)
		Vertex{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 1.0f) * s + fFactor},
		Vertex{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 1.0f) * s + fFactor},
		Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f) * s + fFactor},
		Vertex{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f) * s + fFactor},
	};

	indices = {
		// DOWN
		0, 1, 2,   3,  2,  1,
		// UP
		4,  6,  5,  7,  5,  6,
		// LEFT
		8,  10, 9,  9, 10, 11,
		// RIGHT
		12, 13, 14,  13, 15, 14,
		// FRONT
		16, 17, 18,  17, 19, 18,
		// BACK
		20, 22, 21,  21, 22, 23,
	};

	vertices = CalculateNormalsAndTangents(vertices, indices);
	return CreateMesh(vertices, indices);
}

////////////////////////////////////////////
//// TRIANGLE
////////////////////////////////////////////

Mesh GeometryFactory::BuildTriangle(bool _randomColors)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	vertices = {
		Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3( 0.0f,  0.5f, 0.0f), glm::vec2(0.5f, 0.0f)},
	};
	indices = { 0, 2, 1 };

	vertices = CalculateNormalsAndTangents(vertices, indices);
	if (_randomColors)
		ApplyPaletteColors(vertices);

	return CreateMesh(vertices, indices);
}

////////////////////////////////////////////
//// ICOSPHERE
////////////////////////////////////////////

Mesh GeometryFactory::BuildIcosphere(int _subdivisions, bool _randomColors)
{
	if (_subdivisions >= 6)
		_subdivisions = 6;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	float phi = (0.5f + sqrtf(5.0f)) * 0.5f;

	vertices = {
		Vertex{ glm::vec3(-0.5f,    0,  phi), glm::vec2() },
		Vertex{ glm::vec3(0.5f,    0,  phi), glm::vec2() },
		Vertex{ glm::vec3(-0.5f,    0, -phi), glm::vec2() },
		Vertex{ glm::vec3(0.5f,    0, -phi), glm::vec2() },
		Vertex{ glm::vec3( 0,  phi,    0.5f), glm::vec2() },
		Vertex{ glm::vec3( 0,  phi,   -0.5f), glm::vec2() },
		Vertex{ glm::vec3( 0, -phi,    0.5f), glm::vec2() },
		Vertex{ glm::vec3( 0, -phi,   -0.5f), glm::vec2() },
		Vertex{ glm::vec3( phi,  0.5f,    0), glm::vec2() },
		Vertex{ glm::vec3(-phi,  0.5f,    0), glm::vec2() },
		Vertex{ glm::vec3( phi, -0.5f,    0), glm::vec2() },
		Vertex{ glm::vec3(-phi, -0.5f,    0), glm::vec2() },
	};
	indices = {
		1, 10, 8,   1,  0,  6,   1,  6, 10,   1,  4, 0,   1, 8,  4,
		2, 11, 9,   2,  7, 11,   2,  3,  7,   2,  9, 5,   2, 5,  3,
		3,  5, 8,   3,  8, 10,   3, 10,  7,
		4,  8, 5,   4,  5,  9,   4,  9,  0,
		6, 11, 7,   6,  7, 10,   6,  0, 11,
		11, 0, 9
	};

	for (int i = 0; i < _subdivisions; ++i)
		Subdivide(indices, vertices);

	for (uint32_t i = 0; i < (uint32_t)vertices.size(); ++i)
	{
		glm::vec3 n = glm::normalize(vertices[i].position) * 0.5f;
		vertices[i].position = { n.x, n.y, n.z };

		float u = 0.5f + atan2f(n.z, n.x) / glm::two_pi<float>();
		float v = 0.5f - asinf(n.y) / glm::pi<float>();
		vertices[i].uv = glm::vec2(u, v);
	}

	for (uint32_t i = 0; i < (uint32_t)indices.size(); i += 3)
	{
		uint32_t i0 = indices[i];
		uint32_t i1 = indices[i + 1];
		uint32_t i2 = indices[i + 2];

		float u0 = vertices[i0].uv.x;
		float u1 = vertices[i1].uv.x;
		float u2 = vertices[i2].uv.x;

		if (abs(u0 - u1) > 0.5f || abs(u1 - u2) > 0.5f || abs(u0 - u2) > 0.5f)
		{
			if (u0 < 0.5f)
			{
				vertices.push_back(vertices[i0]);
				vertices.back().uv.x += 1.0f;
				indices[i] = (uint32_t)vertices.size() - 1;
			}
			if (u1 < 0.5f)
			{
				vertices.push_back(vertices[i1]);
				vertices.back().uv.x += 1.0f;
				indices[i + 1] = (uint32_t)vertices.size() - 1;
			}
			if (u2 < 0.5f)
			{
				vertices.push_back(vertices[i2]);
				vertices.back().uv.x += 1.0f;
				indices[i + 2] = (uint32_t)vertices.size() - 1;
			}
		}
	}

	// Step 3: fix pole vertices
	for (uint32_t i = 0; i < (uint32_t)indices.size(); i += 3)
	{
		uint32_t i0 = indices[i];
		uint32_t i1 = indices[i + 1];
		uint32_t i2 = indices[i + 2];

		auto fixPole = [&](uint32_t poleIdx, uint32_t a, uint32_t b) -> uint32_t
		{
			float y = vertices[poleIdx].position.y;
			if (abs(y - 1.0f) < 0.001f || abs(y + 1.0f) < 0.001f)
			{
				vertices.push_back(vertices[poleIdx]);
				vertices.back().uv.x = (vertices[a].uv.x + vertices[b].uv.x) * 0.5f;
				return (uint32_t)vertices.size() - 1;
			}
			return poleIdx;
		};

		indices[i]     = fixPole(i0, i1, i2);
		indices[i + 1] = fixPole(i1, i0, i2);
		indices[i + 2] = fixPole(i2, i0, i1);
	}

	vertices = CalculateNormalsAndTangentsSeamless(vertices, indices);
	if (_randomColors)
		ApplyGradientColors(vertices);

	return CreateMesh(vertices, indices);
}

////////////////////////////////////////////
//// UV SPHERE
////////////////////////////////////////////

Mesh GeometryFactory::BuildUVSphere(uint8_t _sliceCount, uint8_t _stackCount, bool _randomColors)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // top pole
    vertices.push_back(Vertex{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f, 0.0f) });

    float phiStep   = glm::pi<float>() / _stackCount;
    float thetaStep = glm::two_pi<float>() / _sliceCount;

    for (uint32_t i = 1; i < _stackCount; ++i)
    {
        float phi = i * phiStep;
        for (uint32_t j = 0; j <= _sliceCount; ++j)
        {
            float theta = j * thetaStep;
            Vertex v;
            v.position = glm::vec3(
                sinf(phi) * cosf(theta),
                cosf(phi),
                sinf(phi) * sinf(theta)
            );
            v.normal = glm::vec3();
        	v.uv     = glm::vec2(
				(float)j / (float)_sliceCount,
				(float)i / (float)_stackCount
			);
            vertices.push_back(v);
        }
    }

    // bottom pole
    vertices.push_back(Vertex{ glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 1.0f) });

    // top cap - connect pole to first ring
    for (uint32_t i = 1; i <= _sliceCount; ++i)
    {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
    }

    // rings
    uint32_t baseIndex = 1;
    uint32_t ringVertexCount = _sliceCount + 1;
    for (uint32_t i = 0; i < (uint32_t)_stackCount - 2; ++i)
    {
        for (uint32_t j = 0; j < _sliceCount; ++j)
        {
            indices.push_back(baseIndex + i * ringVertexCount + j);
            indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }

    // bottom cap - connect last ring to pole
    uint32_t southPoleIndex = (uint32_t)vertices.size() - 1;
    baseIndex = southPoleIndex - ringVertexCount;
    for (uint32_t i = 0; i < _sliceCount; ++i)
    {
        indices.push_back(southPoleIndex);
        indices.push_back(baseIndex + i);
        indices.push_back(baseIndex + i + 1);
    }

    vertices = CalculateNormalsAndTangentsSeamless(vertices, indices);
	if (_randomColors)
		ApplyGradientColors(vertices);

	return CreateMesh(vertices, indices);
}

////////////////////////////////////////////
//// CIRCLE
////////////////////////////////////////////

Mesh GeometryFactory::BuildCircle(int _stackCount, float _posY, bool _isTop, bool _randomColors)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Circle(vertices, indices, _stackCount, _posY, _isTop);

	if (_isTop == false)
		std::reverse(indices.begin(), indices.end());

	vertices = CalculateNormalsAndTangentsSeamless(vertices, indices);
	if (_randomColors)
		ApplyGradientColors(vertices);

	return CreateMesh(vertices, indices);
}

////////////////////////////////////////////
//// Cylinder
////////////////////////////////////////////

Mesh GeometryFactory::BuildCylinder(int _stackCount, bool _randomColors)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Circle(vertices, indices, _stackCount, -0.5f, false);
	Circle(vertices, indices, _stackCount,  0.5f, true);

	int sideBaseVertex = (int)vertices.size();
	float angle = glm::pi<float>() / (float)_stackCount * 2.0f;
	for (int i = 0; i <= _stackCount; ++i)
	{
		float u  = (float)i / (float)_stackCount;
		float c  = cosf(angle * (float)i);
		float s  = sinf(angle * (float)i);

		vertices.push_back(Vertex{ glm::vec3(c * 0.5f, -0.5f, s * 0.5f), glm::vec2(u, 1.0f) });
		vertices.push_back(Vertex{ glm::vec3(c * 0.5f,  0.5f, s * 0.5f), glm::vec2(u, 0.0f) });
	}

	for (int i = 0; i < _stackCount; ++i)
	{
		int b  = sideBaseVertex + i * 2;
		// bottom-left, top-left, bottom-right
		indices.push_back(b + 0);
		indices.push_back(b + 1);
		indices.push_back(b + 2);
		// top-left, top-right, bottom-right
		indices.push_back(b + 1);
		indices.push_back(b + 3);
		indices.push_back(b + 2);
	}

	vertices = CalculateNormalsAndTangentsSeamless(vertices, indices);
	if (_randomColors)
		ApplyGradientColors(vertices);

	return CreateMesh(vertices, indices);
}

////////////////////////////////////////////
//// Donut
////////////////////////////////////////////

Mesh GeometryFactory::BuildDonut(float _majorRadius, float _minorRadius, uint32_t _majorSlices, uint32_t _minorSlices, bool _randomColors)
{
    std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (uint32_t i = 0; i <= _majorSlices; ++i)
	{
		float u = (float)i / (float)_majorSlices;
		float theta = u * glm::two_pi<float>();

		for (uint32_t j = 0; j <= _minorSlices; ++j)
		{
			float v = (float)j / (float)_minorSlices;
			float phi = v * glm::two_pi<float>();

			float x = (_majorRadius + _minorRadius * cosf(phi)) * cosf(theta);
			float y = _minorRadius * sinf(phi);
			float z = (_majorRadius + _minorRadius * cosf(phi)) * sinf(theta);

			Vertex vertex;
			vertex.position = glm::vec3(x, y, z);
			vertex.uv = glm::vec2(u, v);

			vertices.push_back(vertex);
		}
	}

	for (uint32_t i = 0; i < _majorSlices; ++i)
	{
		for (uint32_t j = 0; j < _minorSlices; ++j)
		{
			uint32_t first = i * (_minorSlices + 1) + j;
			uint32_t second = first + _minorSlices + 1;

			indices.push_back(first + 1);
			indices.push_back(second);
			indices.push_back(first);

			indices.push_back(first + 1);
			indices.push_back(second + 1);
			indices.push_back(second);
		}
	}

	vertices = CalculateNormalsAndTangentsSeamless(vertices, indices);
	if (_randomColors)
		ApplyGradientColors(vertices);

	return CreateMesh(vertices, indices);
}

////////////////////////////////////////////
//// Custom mesh (OBJ)
////////////////////////////////////////////

Mesh GeometryFactory::LoadMesh(std::string const& _path)
{
	std::vector<glm::vec3> rawPositions;
	std::vector<glm::vec2> rawUVs;

	std::vector<Vertex>   vertices;
	std::vector<uint32_t>   indices;

	FILE* file = nullptr;
	errno_t err = fopen_s(&file, _path.c_str(), "r");
	if (err != 0 || file == nullptr)
	{
		// std::cout << "Impossible to open the file: " << _path << '\n';
		return Mesh{};
	}

	while (1)
	{
		char lineHeader[128];
		int res = fscanf_s(file, "%s", lineHeader, (unsigned)_countof(lineHeader));
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "v") == 0)
		{
			glm::vec3 pos;
			fscanf_s(file, "%f %f %f\n", &pos.x, &pos.y, &pos.z);
			rawPositions.push_back(pos);
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			glm::vec2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			// Flip V to match DirectX convention (OBJ origin is bottom-left)
			uv.y = 1.0f - uv.y;
			rawUVs.push_back(uv);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			unsigned int posIdx[3], uvIdx[3], normalIdx[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&posIdx[0], &uvIdx[0], &normalIdx[0],
				&posIdx[1], &uvIdx[1], &normalIdx[1],
				&posIdx[2], &uvIdx[2], &normalIdx[2]);

			if (matches != 9)
			{
				printf("File can't be read by our simple parser\n");
				fclose(file);
				return Mesh{};
			}

			for (int k = 0; k < 3; ++k)
			{
				Vertex v;
				v.position = rawPositions[posIdx[k] - 1];
				v.uv       = rawUVs.empty() ? glm::vec2() : rawUVs[uvIdx[k] - 1];
				v.normal   = glm::vec3();
				indices.push_back((uint32_t)vertices.size());
				vertices.push_back(v);
			}
		}
	}

	fclose(file);

	vertices = CalculateNormalsAndTangents(vertices, indices);

	return CreateMesh(vertices, indices);
}

#pragma endregion

#pragma region HELPERS

void GeometryFactory::ApplyPaletteColors(std::vector<Vertex>& _vertices)
{
	Color const palette[] = {
		Color::RED, Color::GREEN, Color::BLUE, Color::YELLOW,
		Color::CYAN, Color::MAGENTA, Color::ORANGE, Color::PURPLE,
		Color::LIME, Color::PINK, Color::TEAL, Color::BROWN,
	};
	constexpr size_t paletteSize = sizeof(palette) / sizeof(palette[0]);

	for (size_t i = 0; i < _vertices.size(); i++)
		_vertices[i].color = palette[i % paletteSize];
}

void GeometryFactory::ApplyGradientColors(std::vector<Vertex>& _vertices)
{
	if (_vertices.empty())
		return;

	glm::vec3 min = _vertices[0].position;
	glm::vec3 max = _vertices[0].position;
	for (Vertex const& v : _vertices)
	{
		min = glm::min(min, v.position);
		max = glm::max(max, v.position);
	}

	glm::vec3 extent = max - min;
	for (Vertex& v : _vertices)
	{
		glm::vec3 t;
		for (int k = 0; k < 3; k++)
			t[k] = extent[k] > 1e-6f ? (v.position[k] - min[k]) / extent[k] : 0.5f;
		
		v.color = Color(255,
			static_cast<glm::uint8>(55.0f + t.x * 200.0f),
			static_cast<glm::uint8>(55.0f + t.y * 200.0f),
			static_cast<glm::uint8>(55.0f + t.z * 200.0f));
	}
}

std::vector<Vertex> GeometryFactory::CalculateNormalsAndTangentsSeamless(std::vector<Vertex>& _vertices,
    std::vector<Index>& _indices)
{
	struct AccumData
     {
	    glm::vec3 normal = {0.0f, 0.0f, 0.0f };
		glm::vec3 tangent = {0.0f, 0.0f, 0.0f };
     };

	static constexpr float MERGE_EPSILON = 1e-5f;
	
	struct Vec3Hash {
	 	size_t operator()(const glm::vec3& v) const {
	 		auto quantize = [](float f) { return static_cast<int>(std::round(f / MERGE_EPSILON)); };
	 		size_t h1 = std::hash<int>{}(quantize(v.x));
	 		size_t h2 = std::hash<int>{}(quantize(v.y));
	 		size_t h3 = std::hash<int>{}(quantize(v.z));
	 		return h1 ^ (h2 << 16) ^ (h3 << 32);
	 	}
	};

	struct Vec3Eq
	{
	 	bool operator()(const glm::vec3& a, const glm::vec3& b) const {
	 		return std::abs(a.x - b.x) <= MERGE_EPSILON &&
	 			   std::abs(a.y - b.y) <= MERGE_EPSILON &&
	 			   std::abs(a.z - b.z) <= MERGE_EPSILON;
	 	}
	};

	std::unordered_map<glm::vec3, AccumData, Vec3Hash, Vec3Eq> accum;

	uint32_t numTriangle = (uint32_t)_indices.size() / 3;

	for (uint32_t i = 0; i < numTriangle; i++)
	{
	    uint32_t i0 = _indices[i * 3 + 0];
	    uint32_t i1 = _indices[i * 3 + 1];
	    uint32_t i2 = _indices[i * 3 + 2];

	    Vertex& vertex0 = _vertices[i0];
	    Vertex& vertex1 = _vertices[i1];
	    Vertex& vertex2 = _vertices[i2];

	    glm::vec3 e0 = vertex1.position - vertex0.position;
	    glm::vec3 e1 = vertex2.position - vertex0.position;

	    glm::vec3 normal = glm::cross(e0, e1);

	    glm::vec2 deltaUV1 = { vertex1.uv.x - vertex0.uv.x, vertex1.uv.y - vertex0.uv.y };
	    glm::vec2 deltaUV2 = { vertex2.uv.x - vertex0.uv.x, vertex2.uv.y - vertex0.uv.y };

	    glm::vec3 tangent = { 0.0f, 0.0f, 0.0f };
	    float denom = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	    if (std::abs(denom) >= 1e-6f)
	    {
	        float f = 1.0f / denom;
	        glm::vec3 t;
	        tangent.x = f * (deltaUV2.y * e0.x - deltaUV1.y * e1.x);
	        tangent.y = f * (deltaUV2.y * e0.y - deltaUV1.y * e1.y);
	        tangent.z = f * (deltaUV2.y * e0.z - deltaUV1.y * e1.z);
	    }

	    // Accumulate directly into the map for all 3 vertices
	    for (uint32_t idx : { i0, i1, i2 })
	    {
	        AccumData& data = accum[_vertices[idx].position];
	        data.normal += normal;
	        data.tangent += tangent;
	    }
	}

	for (uint32_t i = 0; i < _vertices.size(); i++)
	{
	    AccumData& data = accum[_vertices[i].position];

	    glm::vec3 N = glm::normalize(data.normal);
	    glm::vec3 T = data.tangent;
		
	 	T = T - (glm::dot(T, N) * N);
	 	T = glm::normalize(T);

	    _vertices[i].normal  = N;
	    _vertices[i].tangent = T;
	}

	return _vertices;
}

std::vector<Vertex> GeometryFactory::CalculateNormalsAndTangents(std::vector<Vertex>& _vertices,
	const std::vector<uint32_t>& _indices)
{
	// 1. Utiliser un vecteur simple au lieu d'une map (beaucoup plus rapide)
	struct Accum { glm::vec3 n = {0.0f, 0.0f, 0.0f}; glm::vec3 t = {0.0f, 0.0f, 0.0f}; };
	std::vector<Accum> accum(_vertices.size());
	
	// 2. Premier passage : Accumulation par face
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		uint32_t i0 = _indices[i], i1 = _indices[i+1], i2 = _indices[i+2];
		Vertex& v0 = _vertices[i0]; Vertex& v1 = _vertices[i1]; Vertex& v2 = _vertices[i2];
	
		glm::vec3 p0 = v0.position, p1 = v1.position, p2 = v2.position;
		glm::vec3 e1 = p1 - p0, e2 = p2 - p0;
	
		// Normale de la face (non normalisée pour pondérer par l'aire de la face)
		glm::vec3 faceNormal = glm::cross(e1, e2);
	
		// Tangente
		float du1 = v1.uv.x - v0.uv.x, dv1 = v1.uv.y - v0.uv.y;
		float du2 = v2.uv.x - v0.uv.x, dv2 = v2.uv.y - v0.uv.y;
		float f = 1.0f / (du1 * dv2 - du2 * dv1);
	
		glm::vec3 faceTangent = f * (dv2 * e1 - dv1 * e2);
	
		for (uint32_t idx : {i0, i1, i2}) {
			accum[idx].n += faceNormal;
			accum[idx].t += faceTangent;
		}
	}
	
	// 3. Second passage : Orthogonalisation et stockage
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 N = glm::normalize(accum[i].n);
		glm::vec3 T = accum[i].t;
	
		// Gram-Schmidt : T' = Normalize(T - (N.T) * N)
		T = glm::normalize(T - glm::dot(T, N) * N);
	
		_vertices[i].normal = N;
		// On stocke souvent la tangente en float4 (x, y, z, w) pour la bitangente
		_vertices[i].tangent = T; 
	}
	
	return _vertices;
}

Vertex GeometryFactory::MidPoint(Vertex const& v0, Vertex const& v1)
{
	glm::vec3 pos;
	pos.x = ( v0.position.x + v1.position.x ) * 0.5f;
	pos.y = ( v0.position.y + v1.position.y ) * 0.5f;
	pos.z = ( v0.position.z + v1.position.z ) * 0.5f;

	glm::vec2 uv;
	uv.x = (v0.uv.x + v1.uv.x) * 0.5f;
	uv.y = (v0.uv.y + v1.uv.y) * 0.5f;

	Vertex v = { pos, uv };

	return v;
}

void GeometryFactory::Subdivide(std::vector<uint32_t>& indices, std::vector<Vertex>& vertices)
{
	std::vector<uint32_t> indicesCopy = indices;
	std::vector<Vertex> verticesCopy = vertices;

	vertices.resize(0);
	indices.resize(0);

	uint32_t numTris = static_cast<uint32_t>(indicesCopy.size()) / 3;
	for (uint32_t i = 0; i < numTris; ++i)
	{
		Vertex v0 = verticesCopy[indicesCopy[i * 3 + 0]];
		Vertex v1 = verticesCopy[indicesCopy[i * 3 + 1]];
		Vertex v2 = verticesCopy[indicesCopy[i * 3 + 2]];

		Vertex m0 = MidPoint(v0, v1);
		Vertex m1 = MidPoint(v1, v2);
		Vertex m2 = MidPoint(v0, v2);
			
		vertices.push_back(v0); // 0
		vertices.push_back(v1); // 1
		vertices.push_back(v2); // 2
		vertices.push_back(m0); // 3
		vertices.push_back(m1); // 4
		vertices.push_back(m2); // 5

		indices.push_back(i * 6 + 0);
		indices.push_back(i * 6 + 3);
		indices.push_back(i * 6 + 5);

		indices.push_back(i * 6 + 3);
		indices.push_back(i * 6 + 4);
		indices.push_back(i * 6 + 5);

		indices.push_back(i * 6 + 5);
		indices.push_back(i * 6 + 4);
		indices.push_back(i * 6 + 2);

		indices.push_back(i * 6 + 3);
		indices.push_back(i * 6 + 1);
		indices.push_back(i * 6 + 4);
	}
}

void GeometryFactory::Circle(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, int _stackCount,
	float _posY, bool _isTop)
{
	vertices.push_back(
		Vertex{ glm::vec3(0.0f, _posY, 0.0f), glm::vec2(0.5f, 0.5f) }
	);

	int indexCenter = (int)vertices.size() - 1;
	float angle = glm::pi<float>() / (float)_stackCount * 2.0f;
    	
	for (int i = 0; i < _stackCount + 1; ++i)
	{
		float c = cosf(angle * (float)i);
		float s = sinf(angle * (float)i);
		float posX = c * 0.5f;
		float posZ = s * 0.5f;

		glm::vec2 uv = { 0.5f + c * 0.5f, 0.5f + s * 0.5f };
		vertices.push_back(Vertex{ glm::vec3(posX, _posY, posZ), uv });
	}
	for (int i = indexCenter; i < indexCenter + _stackCount; ++i)
	{
		indices.push_back(indexCenter);
		if (_isTop)
		{
			indices.push_back(i + 2);
			indices.push_back(i + 1);
		}
		else
		{
			indices.push_back(i + 1);
			indices.push_back(i + 2);
		}
	}
}

#pragma endregion
