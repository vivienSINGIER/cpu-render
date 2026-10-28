#pragma once

#include <cassert>
#include <ctime>
#include <iostream>
#include <type_traits>

#include "main.h"
#include "Core/Device.h"
#include "Datas/Texture.h"

#include "Scenes/Scene.h"

union SDL_Event;

class Engine {
public:
	template<typename SCENE>
	void Init(Texture* _frameBuffer);

	void handleEvent(const SDL_Event& event);

	void mainLoop();

	void destroy();

private:
	uint64_t m_lastTime = 0;

	Device m_device;

	Scene* m_mainScene = nullptr;
};

template <typename SCENE>
void Engine::Init(Texture* _frameBuffer)
{
	static_assert(std::is_base_of<Scene, SCENE>::value, "can't init engine with incorrect scene type");

    srand(time(nullptr));
    printf("Init engine \n");

	m_device.SetRenderTarget(_frameBuffer);

	m_mainScene = new SCENE();
	m_mainScene->Init(&m_device);

    std::cout << "End Init" << std::endl;
}
