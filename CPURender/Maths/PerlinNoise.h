#pragma once
#include <glm/gtc/noise.hpp>
#include <vector>

struct FinalFrameBuffer;

//Inspired by https://rtouti.github.io/graphics/perlin-noise-algorithm
struct PerlinNoise
{
public:
	static void InitTable();

	static float Perlin(float x, float y, float frequency);

	//Adding multiple sinusoides of Perlin Noise with more frequency each time to get a fractal effect :)
	static float FractalBrownianMotion(float x, float y, float frequency, int numOctaves);

	//frequency in [0, 1], near 1 = high amount of variations | numOctaves is the amount of iterations for fractal effect -> 1 = classic perlin noise
	static void Generate(FinalFrameBuffer* _output, float frequency = 0.005f, int numOctaves = 1);

private:
	static std::vector<int> m_permutation_table;

	static void Shuffle();
	static float Fade(float t);
	static float Lerp(float a, float b, float t);
	static float Dot(glm::vec2 a, glm::vec2 b);

	static glm::vec2 GetConstantVector(int v);
};
