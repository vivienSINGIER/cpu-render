#include "Octree.h"
#include <iostream>

#include "Maths/Frustum.h"

OctreeNode::OctreeNode()
{
	box = AABB();
	children = {};
	points = {};
	depth = 0;
}

bool OctreeNode::Insert(glm::vec3 point, void* data)
{
	if (box.Contains(point) == false)
		return false;

	if (children.empty())
	{
		if (points.size() < max_capacity || depth >= max_depth)
		{
			points.push_back(Point{ point, data });
			return true;
		}
	}

	// Subdivide
	Subdivide();
	for (Point& p : points)
	{
		for (OctreeNode& o : children)
		{
			if (o.Insert(p.pos, p.data))
				break;
		}

		points.clear();
	}

	// Insert into the appropriate child
	for (OctreeNode& o : children)
	{
		if (o.Insert(point, data))
			return true;
	}

	return false;
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

void OctreeNode::GetDatasInFrustum(Frustum* frustum, std::vector<void*>& result)
{
	//Outside Boxes
	if (!frustum->Intersects(box) && !frustum->Contains(box.Center()))
		return;

	if (children.empty())
	{
		//Check each point
		for (Point& p : points)
		{
			if (!frustum->Contains(p.pos))
				continue;

			result.push_back(p.data);
		}
	}
	else
	{
		//Recurse
		for (OctreeNode& o : children)
		{
			o.GetDatasInFrustum(frustum, result);
		}
	}
}

Octree::Octree(float base_size)
{
	m_base_node.box.SetCenter(glm::vec3(0, 0, 0));
	m_base_node.box.SetExtent(glm::vec3(base_size * 0.5f, base_size * 0.5f, base_size * 0.5f));
}

void Octree::Update()
{
	//TODO REPLACE WITH REAL DATAS
	std::vector<glm::vec3> points;
	const size_t count = 4;

	auto rdm_pos = [&](float min, float max) -> float
		{
			return 1.f;

			return min + (rand() / (double)RAND_MAX) * (max - min);
		};

	const float range = m_base_node.box.Extent().x;

	for (int i = 0; i < count; ++i)
	{
		points.push_back(glm::vec3(rdm_pos(-range, range), rdm_pos(-range, range), rdm_pos(-range, range)));
	}

	for (int i = 0; i < count; ++i)
	{
		std::cout << "Inserting point !\n";
		m_base_node.Insert(points[i], nullptr);
	}

	std::cout << "Cube count = " << total_cube_count << std::endl;
	std::cout << "Deepest Depth = " << deepest_depth << std::endl;
}

std::vector<void*> Octree::GetDatasInFrustum(Frustum* frustum)
{
	std::vector<void*> result;
	m_base_node.GetDatasInFrustum(frustum, result);

	return result;
}

void Octree::SetObjects(std::vector<void*>* objects)
{
	m_objects = objects;
}
