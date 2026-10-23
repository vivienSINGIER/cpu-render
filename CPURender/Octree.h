#pragma once
#include "Maths/AABB.h"
#include <glm/matrix.hpp>
#include <array>

struct Frustum;

struct Point
{
	glm::vec3 pos = glm::vec3(0, 0, 0);
	void* data = nullptr;
};

static constexpr int max_capacity = 3; //need at least more than N datas in the Node to be subdivided
static constexpr int max_depth = 4; //Can't subdivide after the N depth

static size_t total_cube_count = 1;
static size_t deepest_depth = 0;

struct OctreeNode
{
	OctreeNode();

	AABB box;
	std::vector<OctreeNode> children;
	std::vector<Point> points;
	int depth;

	bool Insert(glm::vec3 point, void* data);
	void Subdivide();

	void GetDatasInFrustum(Frustum* frustum, std::vector<void*>& result);
};

class Octree
{
public:
	Octree(float base_size);
	void Update();

	std::vector<void*> GetDatasInFrustum(Frustum* frustum);

	void SetObjects(std::vector<void*>* objects);

private:
	OctreeNode m_base_node;
	std::vector<void*>* m_objects;
};

