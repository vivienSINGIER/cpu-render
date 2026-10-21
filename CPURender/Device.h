#pragma once
#include <glm/matrix.hpp>
#include <vector>

#include "Heaps/Handle.hpp"
#include "Color.h"

#include "Texture.h"

//TODO Replace
struct Camera;
struct GraphicsContext;
struct DrawCommand;
struct Mesh;
struct Material;

class Device
{
public:
	Device();

	void Clear();
	void BeginDraw();
	void Draw(Mesh const& _m, glm::mat4x4 _world, Material const& _mat);
	void EndDraw();

	void ExecCommand();

	void SetMainCamera(Camera* _c);

private:
	Handle m_renderTarget = {};
	Handle m_depthBuffer = {};

	glm::vec4 m_viewport = {};
	Camera* m_mainCam = nullptr;
	Color m_clearColor = {};
	Handle m_defaultConstBuffer = {};
	std::vector<DrawCommand> m_pendingDraws = {};
};

