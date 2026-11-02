#pragma once
#include <array>
#include <glm/matrix.hpp>

#include "Maths/AABB.h"
#include "Maths/Frustum.h"

struct IObject;

struct Point
{
	glm::vec3 pos = glm::vec3(0, 0, 0);
	IObject* data = nullptr;
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

	bool Insert(glm::vec3 point, IObject* data);
	void Subdivide();

	void GetDatasInFrustum(Frustum const& frustum, std::vector<IObject*>& result);

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

	std::vector<IObject*> GetDatasInFrustum(Frustum const& frustum);

	void SetStaticObjects(std::vector<IObject*>* objects);
	void SetDynamicObjects(std::vector<IObject*>* objects);

public:
	static int max_capacity;
	static int max_depth;

private:
	OctreeNode m_static_octree;
	std::vector<IObject*>* m_static_objects;

	OctreeNode m_dynamic_octree;
	std::vector<IObject*>* m_dynamic_objects;
};

