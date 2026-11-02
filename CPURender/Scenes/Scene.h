#pragma once
#include <iostream>
#include <vector>
#include <SDL3/SDL_events.h>

#include "../Camera.h"
#include "../Datas/Object.h"
#include "../Datas/SkinnedObject.h"
#include "../Maths/Transform.h"
#include "../Heaps/Handle.hpp"

#include "../Octree.h"

#include "../main.h"

class Device;

class Scene
{
public:
	Scene();
	virtual ~Scene();

	Object* CreateObject(UpdateObjectType type);
	SkinnedObject* CreateSkinnedObject(const std::string& path);
	void RemoveObject(Object* o);

protected:
	virtual void OnInit(Device* _pDevice) = 0;
	virtual void OnHandleEvent(const SDL_Event& event) {}
	virtual void OnUpdate(float dt) = 0;
	virtual void OnDraw(Device* _pDevice) = 0;

	// Registers every object inside the frustum to the device, to call between BeginPass / EndPass
	void DrawObjects(Device* _pDevice, Frustum const& _frustum, uint8_t _drawMask = 0b11111111);

	Handle m_presentTarget; // texture copied to the screen at the end of the frame, set it in OnInit

	Camera m_camera;
	Transform m_camTransform;

	float m_deltaTime = 0.0f;
	float m_totalTime = 0.0f;

	Device* m_pDevice = nullptr;

private:
	void UpdateCamera(float _dt);
	void RotateCamera(float _yaw, float _pitch);
	void UpdateFps(float _dt);

	bool m_running = true;
	bool dirty_static = true; //update the octree for static objects when new are added or removed

	float m_camYaw = 0.0f;
	float m_camPitch = 0.0f;

	bool m_showFps = true; // toggled with F
	float m_fpsTimer = 0.0f;
	uint32_t m_fpsFrames = 0;

	Octree m_octTree;

	size_t obj_id = 0;

	std::vector<IObject*> m_static_objects = {};
	std::vector<IObject*> m_dynamic_objects = {};

	void Init(Device* _pDevice);
	void HandleEvent(const SDL_Event& event);
	void Update(float dt);
	void Draw(Device* _pDevice);

	friend class Engine;
	friend class Octree;
};

