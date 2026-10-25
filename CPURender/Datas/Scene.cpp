#include "Scene.h"
#include "../Core/Device.h"

Scene::Scene() : m_octTree(1'000.f, 3, 8)
{
	m_octTree.SetStaticObjects(&m_static_objects);
	m_octTree.SetDynamicObjects(&m_dynamic_objects);
}

void Scene::Draw(Device* _device, Frustum const& _frustum)
{
	std::vector<Object*> to_draw = {};

	to_draw = m_octTree.GetDatasInFrustum(_frustum);
	std::cout << "Draw : " << to_draw.size() << std::endl;

	for (Object* o : to_draw)
	{
		_device->RegisterCommand(o->mesh, o->transform.GetMatrix(), o->mat.get());
	}
}

void Scene::Update(float dt)
{
	if (dirty_static)
	{
		dirty_static = false;
		m_octTree.UpdateStatic();
	}

	for (Object* o : m_dynamic_objects)
	{
		o->transform.UpdateMatrix();
		o->mesh.world_box = o->mesh.local_box.Transformed(o->transform.GetMatrix());
	}

	m_octTree.UpdateDynamic();
}

std::vector<Object*>* Scene::GetObjects(ObjectType type)
{
	switch (type)
	{
	case STATIC:
		return &m_static_objects;

	case DYNAMIC:
		return &m_dynamic_objects;
	}

	return nullptr;
}

Object* Scene::CreateObject(ObjectType type)
{
	Object* o = new Object();

	switch (type)
	{
	case STATIC:
		o->id = m_static_objects.size();
		o->type = STATIC;
		m_static_objects.push_back(o);

		dirty_static = true;

		break;

	case DYNAMIC:
		o->id = m_dynamic_objects.size();
		o->type = DYNAMIC;
		m_dynamic_objects.push_back(o);
		break;
	}

	return o;
}

void Scene::RemoveObject(Object* o)
{
	if (o == nullptr)
		return;

	if (o->type == STATIC)
	{
		Object* o2 = m_static_objects.back();
		o2->id = o->id;

		m_static_objects[o->id] = o2;
		m_static_objects.pop_back();

		dirty_static = true;
	}
	else if (o->type == DYNAMIC)
	{
		Object* o2 = m_dynamic_objects.back();
		o2->id = o->id;

		m_dynamic_objects[o->id] = o2;
		m_dynamic_objects.pop_back();
	}
}
