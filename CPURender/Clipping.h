#ifndef CLIPPING_H_DEFINED
#define CLIPPING_H_DEFINED

#include <map>
#include "Datas/Mesh.h"

namespace Clipping
{

	enum FrustumPlane { NEAR = 0, FAR, LEFT, RIGHT, UP, DOWN };
	
	std::pair<Vertex_OUT, FrustumPlane> FindIntersectionWithClip(const Vertex_OUT& v1, const Vertex_OUT& v2);
	
	uint8_t GetFrustumBitmask(const Vertex_OUT& _v);
	
	std::vector<uint8_t> GetBufferFrustumBitmask(std::vector<Vertex_OUT> _vBuffer);
	
	uint8_t GetBufferFrustumBitmaskOr(std::vector<uint8_t> _vBuffer);
	
	float SignedDistanceToPlane(const Vertex_OUT& _v, FrustumPlane _plane);
	
	bool IsFrontOfPlane(const Vertex_OUT& _v, FrustumPlane _plane);
	
	float GetPlaneIntersection(const Vertex_OUT& _v1, const Vertex_OUT& _v2, FrustumPlane _plane);
	
	int ClipPolygonAgainstPlane(Vertex_OUT const* _in, int _inCount, Vertex_OUT* _out, Clipping::FrustumPlane _plane);
}

#endif