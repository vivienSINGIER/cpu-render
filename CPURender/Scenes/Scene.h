#pragma once
#include <iostream>
#include <vector>
#include <SDL3/SDL_events.h>

#include "../Camera.h"
#include "../Datas/Object.h"

#include "../Octree.h"

class Device;

class Scene
{
public:
	Scene();
	virtual ~Scene();

	virtual void Draw(Device* _device, Frustum const& _frustum) = 0;
	void Update(float dt);
	
	Object* CreateObject(ObjectType type);
	void RemoveObject(Object* o);

	void HandleEvent(const SDL_Event& event);
	
protected:
	virtual void OnUpdate(float dt) = 0;
	
private:
	std::vector<Object*>* GetObjects(ObjectType type);
	
	bool dirty_static = true; //update the octree for static objects when new are added or removed

	Octree m_octTree;

	size_t obj_id = 0;

	std::vector<Object*> m_static_objects = {};
	std::vector<Object*> m_dynamic_objects = {};
	
	void OnDraw(Device* _device, Frustum const& _frustum);
	
	friend class Engine;
	friend class Octree;
};

