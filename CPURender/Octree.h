#pragma once
#include <array>
#include <glm/matrix.hpp>

#include "Maths/AABB.h"
#include "Maths/Frustum.h"

struct Object;

struct Point
{
	glm::vec3 pos = glm::vec3(0, 0, 0);
	Object* data = nullptr;
};

static size_t total_cube_count = 1;
static size_t deepest_depth = 0;

struct OctreeNode
{
	OctreeNode();

	AABB box;
	std::vector<OctreeNode> children;
	std::vector<Point> points;
	int depth;

	bool Insert(glm::vec3 point, Object* data);
	void Subdivide();

	void GetDatasInFrustum(Frustum const& frustum, std::vector<Object*>& result);

	void Clear();
};

class Octree
{
public:
	//max_capacity = need at least more than N datas in the Node to be subdivided | max_depth = Can't subdivide after the N depth
	Octree(float base_size, int max_capacity, int max_depth);

	//Call the least possible
	void UpdateStatic();

	//Call every frame
	void UpdateDynamic();

	std::vector<Object*> GetDatasInFrustum(Frustum const& frustum);

	void SetStaticObjects(std::vector<Object*>* objects);
	void SetDynamicObjects(std::vector<Object*>* objects);

public:
	static int max_capacity;
	static int max_depth;

private:
	OctreeNode m_static_octree;
	std::vector<Object*>* m_static_objects;

	OctreeNode m_dynamic_octree;
	std::vector<Object*>* m_dynamic_objects;
};

