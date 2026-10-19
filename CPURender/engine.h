#pragma once

#include "main.h"
#include "Texture.h"
//Students can change whetever they want in this file

union SDL_Event;

struct Engine {

	void init();

	void handleEvent(const SDL_Event& event);

	void renderFrame(Texture& frameBufferOut);

	void destroy();

};