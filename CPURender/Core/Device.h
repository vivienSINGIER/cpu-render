#ifndef DEVICE_H_DEFINED
#define DEVICE_H_DEFINED

#include <glm/matrix.hpp>
#include <vector>

#include "FrameBuffer.hpp"
#include "../Heaps/Handle.hpp"
#include "../Datas/Color.h"

#include "../Datas/Texture.h"
#include "../Datas/DrawCommand.h"

#include "GraphicsContext.h"
#include "../Datas/ConstantBuffers.hpp"

struct LightData;
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
	void Dispatch(MaterialData const* _mat, uint32_t _groupsX, uint32_t _groupsY, uint32_t _groupsZ);
	void Draw();
	void EndDraw();
	
	void Present(Handle _texH);

	void ExecCommand(DrawCommand const& _cmd);
	void ExecCommand(DispatchCommand const& _cmd);

	void SetMainCamera(Camera* _c);
	void SetRenderTarget(Texture* _tex);
	void SetLightData(LightData const& _ld);

private:
	FrameBuffer m_frameBuffer;
	
	Handle m_perPassCB;
	Handle m_lightCB;
	Handle m_perObjectCB;
	Handle m_dispatchCB;
	
	Texture* m_renderTarget;
	
	glm::vec4 m_viewport;
	Color m_clearColor;
	Camera* m_mainCam;
	
	std::vector<Command> m_pendingCmds;
	
	GraphicsContext m_ctx;
	
	std::vector<float> wPreDiv;
	void VertexShaderStep(Mesh const& _mesh, ShaderStages _s, std::vector<Vertex_OUT>& _out);
	
	void ClippingStep(std::vector<Vertex_OUT>& _inOut);
	void ClipToScreen(std::vector<Vertex_OUT>& _inOut);
	
	void RasterStep(std::vector<Vertex_OUT> const& _vBuffer, ShaderStages _s);
};

#endif