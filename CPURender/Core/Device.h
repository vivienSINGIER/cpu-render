#ifndef DEVICE_H_DEFINED
#define DEVICE_H_DEFINED

#include <glm/matrix.hpp>
#include <vector>

#include "../Heaps/Handle.hpp"
#include "../Datas/Color.h"

#include "../Datas/Texture.h"
#include "../Datas/DrawCommand.h"

#include "GraphicsContext.h"
#include "../Datas/ConstantBuffers.hpp"

//TODO Replace
class Camera;
struct Mesh;

class Device
{
public:
	Device();

	void Clear();
	void BeginDraw();
	void RegisterCommand(Mesh const& _m, glm::mat4x4 _world, MaterialData const* _mat);
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
	
	void VertexShaderStep(Mesh const& _mesh, ShaderResources const& _r, ShaderStages _s, std::vector<Vertex_OUT>& _out);
	
	void ClippingStep(std::vector<Vertex_OUT>& _inOut);
	void ClipToScreen(std::vector<Vertex_OUT>& _inOut);
	
	void RasterStep(std::vector<Vertex_OUT> const& _vBuffer, ShaderResources const& _r, ShaderStages _s);
};

#endif