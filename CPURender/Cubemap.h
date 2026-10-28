#pragma once
#include "Datas/Texture.h"
#include "Factories/TextureFactory.h"

class Cubemap
{
	Handle texHandle;

public:
	Cubemap(const char* _name, const char* _front, const char* _back, const char* _left, const char* _right, const char* _up, const char* _down);

	Handle GetTexture();
};

