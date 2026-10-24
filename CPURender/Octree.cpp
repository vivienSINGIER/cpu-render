#include "Octree.h"
#include <iostream>

#include "Maths/Frustum.h"
#include "Datas/Object.h"

OctreeNode::OctreeNode()
{
	box = AABB();
	children = {};
	points = {};
	depth = 0;
}

bool OctreeNode::Insert(glm::vec3 point, Object* data)
{
	if (box.Contains(point) == false)
		return false;

	if (box.Contains(data->mesh.world_box) == false)
		return false;

	if (children.empty())
	{
		if (points.size() < Octree::max_capacity || depth >= Octree::max_depth)
		{
			points.push_back(Point{ point, data });
			return true;
		}

		// Subdivide
		Subdivide();
	}

	//If Subdivided
	std::vector<Point> remainingPoints;
	for (Point& p : points)
	{
		bool insertedToChild = false;
		for (OctreeNode& o : children)
		{
			if (o.Insert(p.pos, p.data))
			{
				insertedToChild = true;
				break;
			}
		}

		//Keep it if no child can contain the point
		if (insertedToChild == false)
		{
			remainingPoints.push_back(p);
		}
	}

	points = std::move(remainingPoints);

	// Insert into the appropriate child
	for (OctreeNode& o : children)
	{
		if (o.Insert(point, data))
			return true;
	}

	//Storage in the parent | no child can contains the object
	points.push_back(Point{ point, data });

	return true;
}

void OctreeNode::Subdivide()
{
	const float parent_half_size = box.Extent().x;
	const float parent_quarter_size = parent_half_size * 0.5f;

	for (int i = -1; i < 2; i += 2)
	{
		for (int j = -1; j < 2; j += 2)
		{
			for (int k = -1; k < 2; k += 2)
			{
				//Adding the 8 childrens
				total_cube_count++;
				children.push_back(OctreeNode());
				OctreeNode& current = children.back();

				glm::vec3 new_pos = box.Center();
				new_pos.x += i * parent_quarter_size;
				new_pos.y += j * parent_quarter_size;
				new_pos.z += k * parent_quarter_size;

				current.box.SetCenter(new_pos);
				current.box.SetExtent(glm::vec3(parent_quarter_size, parent_quarter_size, parent_quarter_size));

				current.depth = depth + 1;
				deepest_depth = current.depth;
			}
		}
	}
}

void OctreeNode::GetDatasInFrustum(Frustum const& frustum, std::vector<Object*>& result)
{
	//Outside Boxes
	if (!frustum.Intersects(box) && !frustum.Contains(box.Center()))
        return;

	//Check each point
	for (Point& p : points)
	{
		if (!frustum.Intersects(p.data->mesh.world_box) && !frustum.Contains(p.data->mesh.world_box.Center()))
		{
			continue;
		}

		result.push_back(p.data);
	}

	//Recurse
	for (OctreeNode& o : children)
	{
		o.GetDatasInFrustum(frustum, result);
	}
}

void OctreeNode::Clear()
{
	points.clear();
	children.clear();
}

int Octree::max_capacity = 0;
int Octree::max_depth = 0;

Octree::Octree(float base_size, int _max_capacity, int _max_depth)
{
	max_capacity = _max_capacity;
	max_depth = _max_depth;

	m_static_objects = nullptr;
	m_dynamic_objects = nullptr;

	m_static_octree.box.SetCenter(glm::vec3(0, 0, 0));
	m_static_octree.box.SetExtent(glm::vec3(base_size * 0.5f, base_size * 0.5f, base_size * 0.5f));

	m_dynamic_octree = m_static_octree;
}

void Octree::UpdateStatic()
{
	m_static_octree.Clear();

	if (m_static_objects == nullptr)
		return;

	for (Object* o : *m_static_objects)
	{
		m_static_octree.Insert(o->transform.GetPosition(), o);
	}
}

void Octree::UpdateDynamic()
{
	m_dynamic_octree.Clear();

	if (m_dynamic_objects == nullptr)
		return;

	for (Object* o : *m_dynamic_objects)
	{
		m_dynamic_octree.Insert(o->transform.GetPosition(), o);
	}
}

std::vector<Object*> Octree::GetDatasInFrustum(Frustum const& frustum)
{
	std::vector<Object*> result;

	m_static_octree.GetDatasInFrustum(frustum, result);
	m_dynamic_octree.GetDatasInFrustum(frustum, result);

	return result;
}

void Octree::SetStaticObjects(std::vector<Object*>* objects)
{
	m_static_objects = objects;
}

void Octree::SetDynamicObjects(std::vector<Object*>* objects)
{
	m_dynamic_objects = objects;
}
