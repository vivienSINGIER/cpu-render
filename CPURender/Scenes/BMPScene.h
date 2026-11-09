#ifndef SCENES_BMP_HPP_DEFINED
#define SCENES_BMP_HPP_DEFINED
#include <SDL3/SDL_events.h>

#include "Scene.h"
#include "../BMP-Parser/BMPParser.hpp"

class Device;

class BMPScene : public Scene
{
public:
    BMPScene();
    
protected:
    void OnInit(Device* _pDevice) override;
    void OnHandleEvent(const SDL_Event& event) override {}
    void OnUpdate(float dt) override {}
    void OnDraw(Device* _pDevice) override {}
    
private:
    BMPParser parser;
};

#endif
