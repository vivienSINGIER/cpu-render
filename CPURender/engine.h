#pragma once

#include "Camera.h"
#include "main.h"
#include "Core/Device.h"
#include "Datas/Texture.h"
//Students can change whetever they want in this file

union SDL_Event;

class Engine {
public:
	void init(Texture* _frameBuffer);

	void handleEvent(const SDL_Event& event);

	void renderFrame();

	void destroy();

private:
	Device m_device;	
	Camera m_camera;
};