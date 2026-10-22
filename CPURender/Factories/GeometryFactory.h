#ifndef GEOMETRY_FACTORY_HPP_INCLUDED
#define GEOMETRY_FACTORY_HPP_INCLUDED

#include <string>
#include <vector>

#include "../Datas/Mesh.h"


class GraphicsContext;

class GeometryFactory
{
public:
	static GeometryFactory* Instance();

	Mesh BuildPyramid();
	Mesh BuildCube();
	Mesh BuildTriangle();
	Mesh BuildIcosphere(int _subdivisions);
	Mesh BuildUVSphere(uint8_t _sliceCount, uint8_t _stackCount);
	Mesh BuildCircle(int _stackCount, float _posY, bool _isTop);
	Mesh BuildCylinder(int _stackCount);
	Mesh BuildDonut(float _majorRadius, float _minorRadius, uint32_t _majorSlices, uint32_t _minorSlices);
	Mesh LoadMesh(std::string const& _path);

private:
	static inline GeometryFactory* g_instance = nullptr;
	GraphicsContext* m_ctx = nullptr;

	GeometryFactory(GraphicsContext* _pContext);
	static void Init(GraphicsContext* _pContext);

	Mesh CreateMesh(std::vector<Vertex>& _vertices, std::vector<Index>& _indices);

	static std::vector<Vertex> CalculateNormalsAndTangentsSeamless(std::vector<Vertex>& _vertices, std::vector<Index>& _indices);

	static std::vector<Vertex> CalculateNormalsAndTangents(std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);
	
	static void ApplyPaletteColors(std::vector<Vertex>& _vertices);
	static void ApplyGradientColors(std::vector<Vertex>& _vertices);

	static Vertex MidPoint(Vertex const& v0, Vertex const& v1);

	static void Subdivide(std::vector<uint32_t>& indices, std::vector<Vertex>& vertices);

	static void Circle(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, int _stackCount, float _posY, bool _isTop);

	friend class GraphicsContext;
};

#endif
