#ifndef CLIPPING_H_DEFINED
#define CLIPPING_H_DEFINED

#include <map>
#include "Datas/Mesh.h"

namespace Clipping{

	enum FrustumPlane { NEAR = 0, FAR, LEFT, RIGHT, UP, DOWN };
	std::pair<Vertex_OUT, FrustumPlane> FindIntersectionWithClip(Vertex_OUT v1, Vertex_OUT v2);
	uint8_t GetFrustumBitmask(Vertex_OUT _v);
	float SignedDistanceToPlane(Vertex_OUT _v, FrustumPlane);
	float GetPlaneIntersection(Vertex_OUT _v1, Vertex_OUT _v2, FrustumPlane _plane);
}

#endif