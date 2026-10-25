#pragma once

#include "Camera.h"
#include "main.h"
#include "Core/Device.h"
#include "Datas/Texture.h"
#include "Datas/Lights/LightData.hpp"
#include "Maths/Transform.h"
#include "Shaders/ShaderBase/Material.h"

union SDL_Event;

class Engine {
public:
	void init(Texture* _frameBuffer);

	void handleEvent(const SDL_Event& event);

	void renderFrame();

	void destroy();

private:
	void UpdateCamera(float _dt);
	void RotateCamera(float _yaw, float _pitch);

	float m_camYaw = 0.0f;
	float m_camPitch = 0.0f;
	uint64_t m_lastTime = 0;
	
	Device m_device;
	Camera m_camera;
	Transform m_camTransform;
	
	Mesh m_mesh;
	Transform m_meshTransform;
	Material m_material;
	Material m_groupDebug;

	LightData m_lightData;
	
	void InitLightScene();
};