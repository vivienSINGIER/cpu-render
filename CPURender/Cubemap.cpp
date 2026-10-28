#include "Cubemap.h"

Cubemap::Cubemap(const char* _name, const char* _front, const char* _back, const char* _left, const char* _right, const char* _up, const char* _down)
{
	// create new texture with 6 combined

	// Get 6 textures
	Texture faceTexs[6];

	faceTexs[0].Load(_front);
	faceTexs[1].Load(_back);
	faceTexs[2].Load(_left);
	faceTexs[3].Load(_right);
	faceTexs[4].Load(_up);
	faceTexs[5].Load(_down);

	// Check if textures are square + check if same size
	int width = faceTexs[0].Width(); int height = faceTexs[0].Height();

	texHandle = TextureFactory::Instance()->CreateTexture(_name, width * 3, height * 3);

	Texture* texture = TextureFactory::Instance()->GetTexture(_name);

	for (int i = 0; i < 6; i++)
	{
		assert(width == faceTexs[i].Width() && height == faceTexs[i].Height(), "Err: All faces must have the same size");
		assert(faceTexs[i].Width() == faceTexs[i].Height(), "Err: All faces must be square");

		int xOffset = (i%2) * width;
		int yOffset = (i/2) * height;

		for (int j = 0; j < height; j++)
		{
			for (int k = 0; k < width; k++)
			{
				texture->SetPixelData(xOffset + k, yOffset + j, faceTexs[i].GetPixelData<vec4>(k, j));
			}
		}
	}
}

Handle Cubemap::GetTexture()
{
	return texHandle;
}

