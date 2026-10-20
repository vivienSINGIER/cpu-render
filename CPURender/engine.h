#pragma once

#include "main.h"
#include "Texture.h"
//Students can change whetever they want in this file
#include "Camera.h"
#include "Object.h"
#include <iostream>

# define M_PI           3.14159265f

union SDL_Event;

struct Engine {

	Camera mainCam;
	Object debugTriangle;
	std::vector<std::shared_ptr<Object>> objects;
	glm::vec4 viewport;

	const double target_frame = 1.0 / 60.0;

	void init();

	void handleEvent(const SDL_Event& event);

	void renderFrame(Texture& frameBufferOut);

	Vertex_OUT VS(Vertex_IN _v, glm::mat4x4 _modelView, glm::mat4x4 _proj);

	bool IsInVP(glm::vec2 _pos);

	void destroy();

};