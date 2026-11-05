#include "Cubemap.h"
#include <chrono>
#include <iostream>

Handle Cubemap::SetTexture(const char* _name, const char* _front, const char* _back, const char* _left, const char* _right, const char* _up, const char* _down, bool _isSrgb, bool _genPBR, uint32_t _irradianceFaceSize)
{
	std::chrono::time_point start = std::chrono::high_resolution_clock::now();
	
	// Get 6 textures
	Texture faceTexs[6];

	faceTexs[0].Load(_front, _isSrgb);
	faceTexs[1].Load(_back, _isSrgb);
	faceTexs[2].Load(_left, _isSrgb);
	faceTexs[3].Load(_right, _isSrgb);
	faceTexs[4].Load(_up, _isSrgb);
	faceTexs[5].Load(_down, _isSrgb);

	// Check if textures are square + check if same size
	int width = faceTexs[0].Width(); 
	int height = faceTexs[0].Height();

	m_texHandle = TextureFactory::Instance()->CreateTexture(_name, width * 3, height * 3);

	Texture* texture = TextureFactory::Instance()->GetTexture(_name);
	
#pragma omp parallel for collapse(3)
	for (int i = 0; i < 6; i++)
	{
		assert(width == faceTexs[i].Width() && height == faceTexs[i].Height(), "Err: All faces must have the same size");
		assert(faceTexs[i].Width() == faceTexs[i].Height(), "Err: All faces must be square");

		int xOffset = (i % 2) * width;
		int yOffset = (i / 2) * height;

		for (int j = 0; j < height; j++)
		{
			for (int k = 0; k < width; k++)
			{
				texture->SetPixelData(xOffset + k, yOffset + j, faceTexs[i].GetPixelData<vec4>(k, j));
			}
		}
	}
	
	texture->GenerateCubeMips();
	
	if (_genPBR)
	{
		std::string irrName = std::string(_name) + "_irradiance";
		m_irradianceHandle = TextureFactory::Instance()->CreateTexture(irrName, _irradianceFaceSize * 3, _irradianceFaceSize * 3);
		Texture* irr = TextureFactory::Instance()->GetTexture(irrName);
	
		Texture::BakeIrradiance(texture, irr, _irradianceFaceSize);
	
		std::string prefName = std::string(_name) + "_prefilter";
		m_prefilterHandle = TextureFactory::Instance()->CreateTexture(prefName, width * 3, height * 3);
		Texture* prefilter = TextureFactory::Instance()->GetTexture(prefName);
	
		Texture::BakePrefilter(texture, prefilter);
	}
	
	std::chrono::time_point end = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration<double>(end - start).count() << " s" << std::endl;
}

Handle Cubemap::GetTexture()
{
	return m_texHandle;
}

Handle Cubemap::GetIrradianceTexture()
{
	return m_irradianceHandle;
}

Handle Cubemap::GetPrefilterTexture()
{
	return m_prefilterHandle;
}
