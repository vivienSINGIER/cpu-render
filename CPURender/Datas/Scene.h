#pragma once
#include <iostream>
#include <vector>

#include "../Camera.h"
#include "Object.h"

#include "../Octree.h"

class Device;

class Scene
{
public:
	Scene();

	void Draw(Device* _device, Frustum const& _frustum);
	void Update(float dt);

	std::vector<Object*>* GetObjects(ObjectType type);
	
	Object* CreateObject(ObjectType type);
	void RemoveObject(Object* o);

private:
	bool dirty_static = true; //update the octree for static objects when new are added or removed

	Octree m_octTree;

	size_t obj_id = 0;

	std::vector<Object*> m_static_objects = {};
	std::vector<Object*> m_dynamic_objects = {};
};

