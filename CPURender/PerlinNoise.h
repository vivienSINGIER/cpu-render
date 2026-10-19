#pragma once
#include <glm/gtc/noise.hpp>
#include <vector>

struct FinalFrameBuffer;

struct PerlinNoise
{
public:
	static void InitTable();

	static float Perlin(float x, float y);

	//TODO Remove when the custom perlin noise is working
	static void GenerateWithGLM(FinalFrameBuffer* _output, float _smoothness);

	static void Generate(FinalFrameBuffer* _output, float _smoothness);

private:
	static std::vector<int> m_permutation_table;

	static void Shuffle();
	static float Fade(float t);
	static float Lerp(float a, float b, float t);
	static float Dot(glm::vec2 a, glm::vec2 b);

	static glm::vec2 GetConstantVector(int v);
};
