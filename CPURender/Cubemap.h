#pragma once
#include "Datas/Texture.h"
#include "Factories/TextureFactory.h"

class Cubemap
{
	Handle m_texHandle;
	Handle m_irradianceHandle;
	Handle m_prefilterHandle;

public:
	Cubemap() {};

	Handle SetTexture(const char* _name,
		const char* _front,
		const char* _back,
		const char* _left,
		const char* _right,
		const char* _up,
		const char* _down,
		bool _isSrgb = false, bool _genPBR = true,
		uint32_t _irradianceFaceSize = 32);
	Handle GetTexture();
	Handle GetIrradianceTexture();
	Handle GetPrefilterTexture();
};

