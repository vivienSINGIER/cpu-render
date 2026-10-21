#ifndef GEOMETRY_FACTORY_HPP_INCLUDED
#define GEOMETRY_FACTORY_HPP_INCLUDED

#include <fstream>
#include <vector>

#include "../Datas/Mesh.h"

#include <glm/geometric.hpp>

#include "../Core/GraphicsContext.h"

struct GeometryFactory
{
public:
	static inline GeometryFactory* Instance();

	static Mesh BuildLine()
	{
		Geometry* pGeometry = _pDevice->CreateGeometry(_isDynamic);

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		vertices = {
			Vertex{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
			Vertex{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)}
		};

		indices = {
			0, 1
		};

		pGeometry->SetVertexData(vertices.data(), 2);
		pGeometry->SetIndexData(indices.data(), 2);

		pGeometry->SetPrimitiveTopology(PrimitiveTopology::LineList);
		
		return pGeometry;
	}
	
	////////////////////////////////////////////
	//// PYRAMID
	////////////////////////////////////////////
	
    static Geometry* BuildPyramid(Device* _pDevice, bool _isDynamic = false)
    {
    	Geometry* pGeometry = _pDevice->CreateGeometry(_isDynamic);

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

    	pGeometry->SetVertexData(vertices.data(), vertices.size());
    	pGeometry->SetIndexData(indices.data(), indices.size());

    	return pGeometry;
    }

	////////////////////////////////////////////
	//// CUBE
	////////////////////////////////////////////
	
	static Geometry* BuildCube(Device* pDevice, bool _isDynamic = false)
    {
		Geometry* pGeometry = pDevice->CreateGeometry(_isDynamic);

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

    	pGeometry->SetVertexData(vertices.data(), vertices.size());
		pGeometry->SetIndexData(indices.data(), indices.size());

		return pGeometry;
    }

	////////////////////////////////////////////
	//// TRIANGLE
	////////////////////////////////////////////
	
	static Geometry* BuildTriangle(Device* _pDevice, bool _isDynamic = false)
    {
		Geometry* pGeometry = _pDevice->CreateGeometry(_isDynamic);

    	std::vector<Vertex> vertices;
    	std::vector<uint32_t> indices;

    	vertices = {
    		Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 1.0f)},
			Vertex{glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 1.0f)},
			Vertex{glm::vec3( 0.0f,  0.5f, 0.0f), glm::vec2(0.5f, 0.0f)},
		};
    	indices = { 0, 2, 1 };

    	vertices = CalculateNormalsAndTangents(vertices, indices);

		pGeometry->SetVertexData(vertices.data(), vertices.size());
		pGeometry->SetIndexData(indices.data(), indices.size());

    	return pGeometry;
    }

    ////////////////////////////////////////////
	//// ICOSPHERE
	////////////////////////////////////////////
	
	static Geometry* BuildIcosphere(Device* _pDevice, int _subdivisions, bool _isDynamic = false)
    {
		Geometry* pGeometry = _pDevice->CreateGeometry(_isDynamic);

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
    		glm::vec3 n = vertices[i].position.Normalized() * 0.5f;
    		vertices[i].position = { n.x, n.y, n.z };
    		
    		float u = 0.5f + atan2f(n.z, n.x) / MathUtils::TWO_PI;
    		float v = 0.5f - asinf(n.y) / MathUtils::PI;
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

		pGeometry->SetVertexData(vertices.data(), vertices.size());
		pGeometry->SetIndexData(indices.data(), indices.size());

		return pGeometry;
    }

	////////////////////////////////////////////
	//// UV SPHERE
	////////////////////////////////////////////

	static Geometry* BuildUVSphere(Device* _pDevice, uint8 _sliceCount, uint8 _stackCount, bool _isDynamic = false)
	{
	    std::vector<Vertex> vertices;
	    std::vector<uint32_t> indices;

	    // top pole
	    vertices.push_back(Vertex{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f, 0.0f) });

	    float phiStep   = MathUtils::PI / _stackCount;
	    float thetaStep = MathUtils::TWO_PI / _sliceCount;

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

    	Geometry* geo = _pDevice->CreateGeometry(_isDynamic);
	    geo->SetVertexData(vertices.data(), vertices.size());
	    geo->SetIndexData(indices.data(), indices.size());
    	return geo;
	}
	
	////////////////////////////////////////////
	//// CIRCLE
	////////////////////////////////////////////
	
	static Geometry* BuildCircle(Device* _pDevice, int _stackCount, float _posY, bool _isTop, bool _isDynamic = false)
    {
    	std::vector<Vertex> vertices;
    	std::vector<uint32_t> indices;

    	Circle(vertices, indices, _stackCount, _posY, _isTop);

    	if (_isTop == false)
    		std::reverse(indices.begin(), indices.end());

    	vertices = CalculateNormalsAndTangentsSeamless(vertices, indices);

		Geometry* geo = _pDevice->CreateGeometry(_isDynamic);
    	
    	geo->SetVertexData(vertices.data(), vertices.size());
    	geo->SetIndexData(indices.data(), indices.size());

    	return geo;
    }

	////////////////////////////////////////////
	//// Cylinder
	////////////////////////////////////////////
	
	static Geometry* BuildCylinder(Device* _pDevice, int _stackCount, bool _isDynamic = false)
    {
    	std::vector<Vertex> vertices;
    	std::vector<uint32_t> indices;

    	Circle(vertices, indices, _stackCount, -0.5f, false);
    	Circle(vertices, indices, _stackCount,  0.5f, true);

    	int sideBaseVertex = (int)vertices.size();
    	float angle = MathUtils::PI / (float)_stackCount * 2.0f;
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

    	vertices = CalculateNormalsAndTangents(vertices, indices);

		Geometry* geo = _pDevice->CreateGeometry(_isDynamic);
    	
    	geo->SetVertexData(vertices.data(), vertices.size());
    	geo->SetIndexData(indices.data(), indices.size());

    	return geo;
    }

	////////////////////////////////////////////
	//// Donut
	////////////////////////////////////////////
	
	static Geometry* BuildDonut(Device* _pDevice, float _majorRadius, float _minorRadius, uint32_t _majorSlices, uint32_t _minorSlices, bool _isDynamic = false)
    {
	    std::vector<Vertex> vertices;
    	std::vector<uint32_t> indices;

    	for (uint32_t i = 0; i <= _majorSlices; ++i)
    	{
			float u = (float)i / (float)_majorSlices;
    		float theta = u * 2.0f * MathUtils::PI;
    		
    		for (uint32_t j = 0; j <= _minorSlices; ++j)
    		{
    			float v = (float)j / (float)_minorSlices;
    			float phi = v * 2.0f * MathUtils::PI;

    			float x = (_majorRadius + _minorRadius * MathUtils::Cos(phi)) * MathUtils::Cos(theta);
    			float y = _minorRadius * MathUtils::Sin(phi);
    			float z = (_majorRadius + _minorRadius * MathUtils::Cos(phi)) * MathUtils::Sin(theta);

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

    	Geometry* geo = _pDevice->CreateGeometry(_isDynamic);
    	
    	geo->SetVertexData(vertices.data(), vertices.size());
    	geo->SetIndexData(indices.data(), indices.size());

    	return geo;
    }
	
	////////////////////////////////////////////
	//// GeometryCustom
	////////////////////////////////////////////

	static Geometry* LoadGeometry(Device* _pDevice, std::string _path, bool _isDynamic = false)
    {
		Geometry* pGeo = _pDevice->CreateGeometry(_isDynamic);
    	
    	std::vector<glm::vec3> rawPositions;
    	std::vector<glm::vec2> rawUVs;
    	
    	std::vector<Vertex>   vertices;
    	std::vector<uint32_t>   indices;

    	FILE* file = nullptr;
    	errno_t err = fopen_s(&file, _path.c_str(), "r");
    	if (err != 0 || file == nullptr)
    	{
    		// std::cout << "Impossible to open the file: " << _path << '\n';
    		return nullptr;
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
    				return nullptr;
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

    	pGeo->SetVertexData(vertices.data(), vertices.size());
    	pGeo->SetIndexData(indices.data(), indices.size());

		return pGeo;
    }
	
	static Geometry* LoadJsonGeometry(Device* _pDevice, json const& object)
    {
    	Geometry* pGeo = _pDevice->CreateGeometry();

    	std::vector<Vertex> vertices;
    	std::vector<uint32_t> indices;

    	LoadJsonObj(object, vertices, indices);
    	vertices = CalculateNormalsAndTangents(vertices, indices);
    	
    	pGeo->SetVertexData(vertices.data(), vertices.size());
    	pGeo->SetIndexData(indices.data(), indices.size());

    	return pGeo;
    }
	
private:
	static inline GeometryFactory* g_instance = nullptr;
	GraphicsContext* m_ctx = nullptr;
	
	GeometryFactory();
	void SetContext(GraphicsContext* _pContext);
	
	static std::vector<Vertex> CalculateNormalsAndTangentsSeamless(std::vector<Vertex>& _vertices, std::vector<Index>& _indices);

	static std::vector<Vertex> CalculateNormalsAndTangents(std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);

	static Vertex MidPoint(Vertex const& v0, Vertex const& v1);

	static void Subdivide(std::vector<uint32_t>& indices, std::vector<Vertex>& vertices);

	static void Circle(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, int _stackCount, float _posY, bool _isTop);
};

#endif