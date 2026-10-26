#include "Scene.h"
#include "../Core/Device.h"

#include <SDL3/SDL_keyboard.h>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>

namespace
{
	constexpr float CAM_MOVE_SPEED = 3.0f;          // world units per second
	constexpr float CAM_KEY_ROT_SPEED = 1.5f;       // radians per second
	constexpr float CAM_MOUSE_SENSITIVITY = 0.005f; // radians per pixel
	constexpr float CAM_MAX_PITCH = glm::radians(89.0f);
	constexpr float MAX_DELTA_TIME = 0.1f;          // avoids a big jump after a hitch

	const glm::vec3 WORLD_UP = { 0.0f, 1.0f, 0.0f };
}

Scene::Scene() : m_octTree(1'000.f, 3, 8)
{
	m_octTree.SetStaticObjects(&m_static_objects);
	m_octTree.SetDynamicObjects(&m_dynamic_objects);
}

Scene::~Scene()
{
	for (Object* o : m_dynamic_objects)
	{
		delete o;
	}
	for (Object* o : m_static_objects)
	{
		delete o;
	}
}

void Scene::Init(Device* _pDevice)
{
	m_camera.SetAspectRatio((float)WIDTH / (float)HEIGHT);

	OnInit(_pDevice);
}

void Scene::Draw(Device* _pDevice)
{
	OnDraw(_pDevice);
}

void Scene::DrawObjects(Device* _device, Frustum const& _frustum)
{
	std::vector<Object*> to_draw = {};

	to_draw = m_octTree.GetDatasInFrustum(_frustum);

	for (Object* o : to_draw)
	{
		_device->RegisterCommand(o->mesh, o->transform.GetMatrix(), o->mat);
	}
}

void Scene::Update(float dt)
{
	m_deltaTime = dt;
	m_totalTime += dt;

	UpdateCamera(glm::min(dt, MAX_DELTA_TIME));
	if (m_camTransform.IsWorldDirty())
	{
		m_camera.SetWorld(m_camTransform.GetMatrix());
	}

	OnUpdate(dt);
	
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

void Scene::HandleEvent(const SDL_Event& event)
{
	if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_ESCAPE))
	{
		m_running = false;
	}
	if (event.type == SDL_EVENT_MOUSE_MOTION && (event.motion.state & SDL_BUTTON_RMASK))
	{
		RotateCamera(event.motion.xrel * CAM_MOUSE_SENSITIVITY, event.motion.yrel * CAM_MOUSE_SENSITIVITY);
	}

	OnHandleEvent(event);
}

void Scene::RotateCamera(float _yaw, float _pitch)
{
	m_camYaw += _yaw;
	m_camPitch = glm::clamp(m_camPitch + _pitch, -CAM_MAX_PITCH, CAM_MAX_PITCH);

	glm::quat yaw = glm::angleAxis(m_camYaw, WORLD_UP);
	glm::quat pitch = glm::angleAxis(m_camPitch, glm::vec3(1.0f, 0.0f, 0.0f));

	m_camTransform.SetRotationQuaternion(glm::normalize(yaw * pitch));
}

void Scene::UpdateCamera(float _dt)
{
	bool const* keys = SDL_GetKeyboardState(nullptr);

	glm::vec3 move = { 0.0f, 0.0f, 0.0f };
	if (keys[SDL_SCANCODE_W])     move += m_camTransform.GetForward();
	if (keys[SDL_SCANCODE_S])     move -= m_camTransform.GetForward();
	if (keys[SDL_SCANCODE_D])     move += m_camTransform.GetRight();
	if (keys[SDL_SCANCODE_A])     move -= m_camTransform.GetRight();
	if (keys[SDL_SCANCODE_SPACE]) move += WORLD_UP;
	if (keys[SDL_SCANCODE_LCTRL]) move -= WORLD_UP;

	if (glm::dot(move, move) > 0.0f)
		m_camTransform.Move(glm::normalize(move), CAM_MOVE_SPEED * _dt);

	float yaw = 0.0f;
	float pitch = 0.0f;
	if (keys[SDL_SCANCODE_LEFT])  yaw -= 1.0f;
	if (keys[SDL_SCANCODE_RIGHT]) yaw += 1.0f;
	if (keys[SDL_SCANCODE_UP])    pitch -= 1.0f;
	if (keys[SDL_SCANCODE_DOWN])  pitch += 1.0f;

	if (yaw != 0.0f || pitch != 0.0f)
		RotateCamera(yaw * CAM_KEY_ROT_SPEED * _dt, pitch * CAM_KEY_ROT_SPEED * _dt);
}
