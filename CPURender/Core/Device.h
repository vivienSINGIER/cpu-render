#ifndef DEVICE_H_DEFINED
#define DEVICE_H_DEFINED

#include <glm/matrix.hpp>
#include <vector>

#include "Binning.hpp"
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
	GraphicsContext m_ctx;

	Device();
	
	void BeginFrame();
	void BeginPass(PassDesc const& _pass);
	
	void EndPass();
	void EndFrame();
	
	void Clear(FrameBuffer _fb, ClearFlags _flags);
	void RegisterCommand(Mesh const& _m, glm::mat4x4 _world, MaterialData const* _mat, std::vector<glm::mat4x4> const& _finalBoneMatrices = {});
	void Dispatch(MaterialData const* _mat, uint32_t _groupsX, uint32_t _groupsY, uint32_t _groupsZ);
	
	void Present(Handle _texH);

	void ExecCommand(DrawCommand const& _cmd);
	void ExecCommand(DispatchCommand const& _cmd);
	
	void SetRenderTarget(Texture* _tex);
	void SetLightData(LightData const& _ld);
	void SetExposure(float _exposure) { m_exposure = _exposure; }
	void SetAutoExposure(bool _enabled) { m_autoExposure = _enabled; }

	FrameBuffer CreateFrameBuffer(uint32_t _w, uint32_t _h, uint32_t _colorBuffCount, bool _useDepth, glm::vec4 _clearColor = vec4(0.1f, 0.1f, 0.1f, 1.0f));
	FrameBuffer const& GetDefaultFrameBuffer();

private:
	FrameBuffer m_defaultFB;
	FrameBuffer m_boundFB;
	
	glm::vec4 m_viewport;
	Binning::Tiles m_tiles;
	std::vector<RenderPass> m_pendingPasses;
	bool m_hasOpenPass = false;
	uint32_t m_activePass = 0;
	
	Handle m_perPassCB;
	Handle m_lightCB;
	Handle m_perObjectCB;
	Handle m_dispatchCB;
	
	Texture* m_renderTarget;
	
	float m_adaptedLum = 0.18f;
	float m_exposure = 1.0f;
	bool m_autoExposure = true;
	
	std::vector<Vertex_OUT> m_transformed;
	std::vector<uint8> m_visible;
	std::vector<Vertex_OUT> m_clipped;
	std::vector<float> wPreDiv;
	
	void VertexShaderStep(Mesh const& _mesh, ShaderStages _s);
	void ClippingStep(Mesh const& _mesh);
	void ClipToScreen();
	void RasterStep(ShaderStages _s);
};

#endif