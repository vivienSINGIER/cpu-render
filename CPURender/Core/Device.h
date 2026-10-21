#ifndef DEVICE_H_DEFINED
#define DEVICE_H_DEFINED

#include <glm/matrix.hpp>
#include <vector>

#include "../Heaps/Handle.hpp"
#include "../Datas/Color.h"

#include "../Datas/Texture.h"
#include "../Datas/DrawCommand.h"

#include "GraphicsContext.h"

//TODO Replace
struct Camera;
struct Mesh;
struct Material;

class Device
{
public:
	Device();

	void Clear();
	void BeginDraw();
	void RegisterCommand(Mesh const& _m, glm::mat4x4 _world, Material const& _mat);
	void Draw();
	void EndDraw();

	void ExecCommand(DrawCommand const& _cmd);

	void SetMainCamera(Camera* _c);
	void SetRenderTarget(Texture* _tex);

private:
	Handle m_frameBuffer;
	Handle m_depthBuffer;

	Handle m_perPassCB;
	Handle m_perObjectCB;
	
	Texture* m_renderTarget;
	
	glm::vec4 m_viewport;
	Color m_clearColor;
	Camera* m_mainCam;
	
	std::vector<DrawCommand> m_pendingDraws;
	
	GraphicsContext m_ctx;
};

#endif