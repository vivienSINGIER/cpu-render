#include "Scene.h"
#include "../Core/Device.h"

#include <SDL3/SDL_keyboard.h>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>

namespace
{
	constexpr float CAM_MOVE_SPEED = 10.0f;          // world units per second
	constexpr float CAM_KEY_ROT_SPEED = 1.5f;       // radians per second
	constexpr float CAM_MOUSE_SENSITIVITY = 0.005f; // radians per pixel
	constexpr float CAM_MAX_PITCH = glm::radians(89.0f);
	constexpr float MAX_DELTA_TIME = 0.1f;          // avoids a big jump after a hitch
	constexpr float FPS_REFRESH_TIME = 1.0f;        // seconds between two fps prints

	const glm::vec3 WORLD_UP = { 0.0f, 1.0f, 0.0f };
}

Scene::Scene() : m_octTree(1'000.f, 3, 8)
{
	m_octTree.SetStaticObjects(&m_static_objects);
	m_octTree.SetDynamicObjects(&m_dynamic_objects);
}

Scene::~Scene()
{
	for (IObject* o : m_dynamic_objects)
	{
		delete o;
	}
	for (IObject* o : m_static_objects)
	{
		delete o;
	}
	m_dynamic_objects.clear();
	m_static_objects.clear();
}

void Scene::Init(Device* _pDevice)
{
	m_pDevice = _pDevice;
	m_camera.SetAspectRatio((float)WIDTH / (float)HEIGHT);

	OnInit(_pDevice);
}

void Scene::Draw(Device* _pDevice)
{
	OnDraw(_pDevice);
}

void Scene::DrawObjects(Device* _device, Frustum const& _frustum, uint8_t _drawMask)
{
	std::vector<IObject*> to_draw = {};

	to_draw = m_octTree.GetDatasInFrustum(_frustum);

	for (IObject* o : to_draw)
	{
		if (_drawMask & o->drawMask)
		{
			if (o->obj_type == ObjectType::SKINNED)
			{
				SkinnedObject* skin = (SkinnedObject*)o;
				_device->RegisterCommand(skin->mesh, skin->transform.GetMatrix(), skin->mat, skin->animator.GetFinalBoneMatrices());
			}
			else
				_device->RegisterCommand(o->mesh, o->transform.GetMatrix(), o->mat);
		}
	}
}

void Scene::Update(float dt)
{
	m_deltaTime = dt;
	m_totalTime += dt;

	UpdateFps(dt);
	UpdateCamera(glm::min(dt, MAX_DELTA_TIME));
	if (m_camTransform.IsWorldDirty())
	{
		m_camera.SetWorld(m_camTransform.GetMatrix());
	}

	OnUpdate(dt);
	
	if (dirty_static)
	{
		dirty_static = false;                                                                                                                                                                                                                             
		for (IObject* o : m_static_objects)                                                                                                                                                                                                                
		{                                                                                                                                                                                                                                                 
			o->transform.UpdateMatrix();                                                                                                                                                                                                              
			o->mesh.world_box = o->mesh.local_box.Transformed(o->transform.GetMatrix());                                                                                                                                                              
		}                                                                                                                                                                                                                                                 
		m_octTree.UpdateStatic(); 
	}

	bool dirty = false;
	for (IObject* o : m_dynamic_objects)
	{
		if (o->obj_type == ObjectType::SKINNED)
		{
			SkinnedObject* skin = (SkinnedObject*)o;
			skin->animator.UpdateAnimation(dt);
		}

		if (o->transform.IsWorldDirty() == false)
			continue;
	
		dirty = true;
		o->transform.UpdateMatrix();
		o->mesh.world_box = o->mesh.local_box.Transformed(o->transform.GetMatrix());
	}

	if (dirty)
	{
		dirty = false;
		m_octTree.UpdateDynamic();
	}
}

Object* Scene::CreateObject(UpdateObjectType type)
{
	Object* o = new Object();

	switch (type)
	{
	case STATIC:
		o->id = m_static_objects.size();
		o->update_type = STATIC;
		m_static_objects.push_back(o);

		dirty_static = true;

		break;

	case DYNAMIC:
		o->id = m_dynamic_objects.size();
		o->update_type = DYNAMIC;
		m_dynamic_objects.push_back(o);
		break;
	}

	return o;
}

SkinnedObject* Scene::CreateSkinnedObject(const std::string& path)
{
	SkinnedObject* o = new SkinnedObject(path);
	m_dynamic_objects.push_back(o);

	return o;
}

void Scene::RemoveObject(Object* o)
{
	if (o == nullptr)
		return;

	if (o->update_type == STATIC)
	{
		IObject* o = m_static_objects.back();
		o->id = o->id;

		m_static_objects[o->id] = o;
		m_static_objects.pop_back();

		dirty_static = true;
		obj_id--;
	}
	else if (o->update_type == DYNAMIC)
	{
		IObject* o = m_dynamic_objects.back();
		o->id = o->id;

		m_dynamic_objects[o->id] = o;
		m_dynamic_objects.pop_back();
		obj_id--;
	}
}

void Scene::HandleEvent(const SDL_Event& event)
{
	if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_ESCAPE))
	{
		m_running = false;
	}
	if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F)
	{
		m_showFps = !m_showFps;
		m_fpsTimer = 0.0f;
		m_fpsFrames = 0;
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

void Scene::UpdateFps(float _dt)
{
	if (m_showFps == false)
		return;

	// Averaged over FPS_REFRESH_TIME so the console isn't flooded every frame
	m_fpsTimer += _dt;
	m_fpsFrames++;

	if (m_fpsTimer >= FPS_REFRESH_TIME)
	{
		std::cout << "FPS : " << (float)m_fpsFrames / m_fpsTimer << " (" << 1000.0f * m_fpsTimer / (float)m_fpsFrames << " ms)\n";
		m_fpsTimer = 0.0f;
		m_fpsFrames = 0;
	}
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
}
