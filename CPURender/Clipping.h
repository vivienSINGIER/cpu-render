#ifndef CLIPPING_H_DEFINED
#define CLIPPING_H_DEFINED

#include <map>
#include "Datas/Mesh.h"

namespace Clipping{

	enum FrustumPlane { NEAR = 0, FAR, LEFT, RIGHT, UP, DOWN };
	std::pair<Vertex_OUT, FrustumPlane> FindIntersectionWithClip(Vertex_OUT v1, Vertex_OUT v2);
	uint8_t GetFrustumBitmask(Vertex_OUT _v);
	std::vector<uint8_t> GetBufferFrustumBitmask(std::vector<Vertex_OUT> _vBuffer);
	uint8_t GetBufferFrustumBitmaskOr(std::vector<uint8_t> _vBuffer);
	float SignedDistanceToPlane(Vertex_OUT _v, FrustumPlane _plane);
	bool IsFrontOfPlane(Vertex_OUT _v, FrustumPlane _plane);
	float GetPlaneIntersection(Vertex_OUT _v1, Vertex_OUT _v2, FrustumPlane _plane);
}

#endif