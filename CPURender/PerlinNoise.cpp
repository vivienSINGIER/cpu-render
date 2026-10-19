#include <iostream>
#include "PerlinNoise.h"
#include <Windows.h>

#include "main.h"

std::vector<int> PerlinNoise::m_permutation_table = {};

float PerlinNoise::Perlin(float x, float y)
{
	int X = (int)std::floor(x) & 255;
	int Y = (int)std::floor(y) & 255;

	float xf = 1.f - std::floor(x);
	float yf = 1.f - std::floor(y);

	const glm::vec2 topRight = glm::vec2(xf - 1.f, yf - 1.f);
	const glm::vec2 topLeft = glm::vec2(xf, yf - 1.f);
	const glm::vec2 bottomRight = glm::vec2(xf - 1.f, yf);
	const glm::vec2 bottomLeft = glm::vec2(xf, yf);

	const int valueTopRight = m_permutation_table[m_permutation_table[X + 1] + Y + 1];
	const int valueTopLeft = m_permutation_table[m_permutation_table[X] + Y + 1];
	const int valueBottomRight = m_permutation_table[m_permutation_table[X + 1] + Y];
	const int valueBottomLeft = m_permutation_table[m_permutation_table[X] + Y];

	const float dotTopRight = Dot(topRight, GetConstantVector(valueTopRight));
	const float dotTopLeft = Dot(topRight, GetConstantVector(valueTopLeft));
	const float dotBottomRight = Dot(topRight, GetConstantVector(valueBottomRight));
	const float dotBottomLeft = Dot(topRight, GetConstantVector(valueBottomLeft));

	const float u = Fade(xf);
	const float v = Fade(yf);

	float result = Lerp(u, Lerp(v, dotBottomLeft, dotTopLeft), Lerp(v, dotBottomRight, dotTopRight));

	return result;
}

void PerlinNoise::GenerateWithGLM(FinalFrameBuffer* _output, float _smoothness)
{
	if (_smoothness == 0)
	{
		std::cerr << "Intensity can't be 0 for Perlin Noise";
		return;
	}

	//src = https://stackoverflow.com/questions/13488957/interpolate-from-one-color-to-another
	auto interpolate = [&](int color1, int color2, float fraction)
		{
			unsigned char   r1 = (color1 >> 16) & 0xff;
			unsigned char   r2 = (color2 >> 16) & 0xff;
			unsigned char   g1 = (color1 >> 8) & 0xff;
			unsigned char   g2 = (color2 >> 8) & 0xff;
			unsigned char   b1 = color1 & 0xff;
			unsigned char   b2 = color2 & 0xff;

			return (int)((r2 - r1) * fraction + r1) << 16 |
				(int)((g2 - g1) * fraction + g1) << 8 |
				(int)((b2 - b1) * fraction + b1);
		};
	//

	auto gen_color = [&](float value) -> uint32_t
		{
			uint32_t blue = 0x0000FF;
			uint32_t cyan = 0x00C3FF;

			uint32_t yellow = 0xFFF87A;
			uint32_t green = 0x0A9600;

			uint32_t gray = 0x474747;
			uint32_t white = 0xFFFFFF;

			uint32_t color = {};

			float barycenter = 0.f;

			if (value >= 0.f && value < 0.3f) //Blue to cyan
			{
				barycenter = value / 0.3f;

				return interpolate(blue, cyan, barycenter);
			}
			else if (value >= 0.3f && value < 0.7f) //Yellow to Green
			{
				barycenter = (value - 0.3f) / (0.7f - 0.3f);

				return interpolate(yellow, green, barycenter);
			}
			else //Gray to white
			{
				barycenter = (value - 0.7f) / (1.f - 0.7f);

				return interpolate(gray, white, barycenter);
			}
		};

	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			glm::vec2 pos = { (float)x, (float)y};

			float noise_val = glm::perlin(pos / _smoothness);
			noise_val = (noise_val + 1.f) / 2.f;

			uint32_t final_color = gen_color(noise_val);

			_output->put_pixel(x, y, final_color);
		}
	}
}

void PerlinNoise::Generate(FinalFrameBuffer* _output, float _smoothness)
{
	//src = https://stackoverflow.com/questions/13488957/interpolate-from-one-color-to-another
	auto interpolate = [&](int color1, int color2, float fraction)
		{
			unsigned char   r1 = (color1 >> 16) & 0xff;
			unsigned char   r2 = (color2 >> 16) & 0xff;
			unsigned char   g1 = (color1 >> 8) & 0xff;
			unsigned char   g2 = (color2 >> 8) & 0xff;
			unsigned char   b1 = color1 & 0xff;
			unsigned char   b2 = color2 & 0xff;

			return (int)((r2 - r1) * fraction + r1) << 16 |
				(int)((g2 - g1) * fraction + g1) << 8 |
				(int)((b2 - b1) * fraction + b1);
		};
	//

	auto gen_color = [&](float value) -> uint32_t
		{
			uint32_t blue = 0x0000FF;
			uint32_t cyan = 0x00C3FF;

			uint32_t yellow = 0xFFF87A;
			uint32_t green = 0x0A9600;

			uint32_t gray = 0x474747;
			uint32_t white = 0xFFFFFF;

			uint32_t color = {};

			float barycenter = 0.f;

			if (value >= 0.f && value < 0.3f) //Blue to cyan
			{
				barycenter = value / 0.3f;

				return interpolate(blue, cyan, barycenter);
			}
			else if (value >= 0.3f && value < 0.7f) //Yellow to Green
			{
				barycenter = (value - 0.3f) / (0.7f - 0.3f);

				return interpolate(yellow, green, barycenter);
			}
			else //Gray to white
			{
				barycenter = (value - 0.7f) / (1.f - 0.7f);

				return interpolate(gray, white, barycenter);
			}
		};

	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			glm::vec2 pos = { (float)x, (float)y };

			float noise_val = Perlin(pos.x / _smoothness, pos.y / _smoothness);
			noise_val = (noise_val + 1.f) / 2.f;

			uint32_t final_color = gen_color(noise_val);

			_output->put_pixel(x, y, final_color);
		}
	}
}

void PerlinNoise::InitTable()
{
	srand(time(nullptr));

	for (int i = 0; i < 256; ++i)
	{
		m_permutation_table.push_back(i);
	}

	Shuffle();

	//to 512
	for (int i = 0; i < 256; ++i)
	{
		m_permutation_table.push_back(m_permutation_table[i]);
	}
}

void PerlinNoise::Shuffle()
{
	for (int i = m_permutation_table.size() - 1; i > 0; i--)
	{
		const int index = rand() % i;
		const int tmp_val = m_permutation_table[i];

		m_permutation_table[i] = m_permutation_table[index];
		m_permutation_table[index] = tmp_val;
	}
}

float PerlinNoise::Fade(float t)
{
	//6t^5 -15t^4 +10t^3 optimized
	return ((6 * t - 15) * t + 10) * t * t * t;
}

float PerlinNoise::Lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

float PerlinNoise::Dot(glm::vec2 a, glm::vec2 b)
{
	return a.x*b.x + a.y*b.y;
}

glm::vec2 PerlinNoise::GetConstantVector(int v)
{
	int h = v % 3;

	switch (h)
	{
	case 0:
		return glm::vec2(1.f, 1.f);
	case 1:
		return glm::vec2(-1.f, 1.f);
	case 2:
		return glm::vec2(-1.f, -1.f);
	default:
		return glm::vec2(1.f, -1.f);
	}
}
