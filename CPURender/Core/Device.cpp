#include "Device.h"

#include <iostream>
#include <ostream>

#include "Binning.hpp"
#include "../Camera.h"
#include "../main.h"
#include "../Datas/DrawCommand.h"
#include "../Datas/ConstantBuffers.hpp"
#include "../Datas/Triangle.h"
#include "../Clipping.h"
#include "../Datas/Lights/LightData.hpp"
#include "../Shaders/ShaderBase/Material.h"

Device::Device() : m_ctx()
{
	m_renderTarget = nullptr;
	
	m_defaultFB.width = WIDTH;
	m_defaultFB.height = HEIGHT;
	m_defaultFB.gBuffer[0] = m_ctx.textureHeap.Create<Texture>(WIDTH, HEIGHT, Texture::RGBA32, Texture::MORTON, Color::WHITE);
	m_defaultFB.depthBuffer = m_ctx.textureHeap.Create<Texture>(WIDTH, HEIGHT, Texture::R32F, Texture::MORTON, Color::BLACK);
	m_defaultFB.clearColor = vec4(0.1f, 0.1, 0.1, 1.0f);
	
	m_perPassCB = m_ctx.constantBufferHeap.Create(sizeof(PerPassCB));
	m_lightCB = m_ctx.constantBufferHeap.Create(sizeof(LightPassCB));
	m_perObjectCB = m_ctx.constantBufferHeap.Create(sizeof(PerObjectCB));
	m_dispatchCB = m_ctx.constantBufferHeap.Create(sizeof(DispatchCB));
}

void Device::BeginFrame()
{
	assert(m_renderTarget != nullptr && "Can't draw on uninitialized render target");
	
	m_pendingPasses.clear();
	
	m_ctx.SetHeapsState(HeapState::BLOCKED);
}

void Device::BeginPass(PassDesc const& _pass)
{
	assert(m_hasOpenPass == false && "Can't open multiple render passes at once");
	m_hasOpenPass = true;
	
	m_pendingPasses.push_back({_pass, {}});
}

void Device::Clear(FrameBuffer _fb, ClearFlags _flags)
{
	for (int i = 0; i < MAX_RT; i++)
	{
		if (_fb.gBuffer[i].IsValid() == false)
			continue;
		if (_flags.clearGBuffer[i] == false)
			continue;
		
		Texture* rt = m_ctx.textureHeap.Get(_fb.gBuffer[i]);
		rt->Clear(_fb.clearColor);
	}
	
	if (_fb.useDepth == false)
		return;
	if (_flags.clearDepth == false)
		return;
	
	Texture* depth = m_ctx.textureHeap.Get(_fb.depthBuffer);
	depth->Clear(glm::vec4(1.0f));
}

void Device::RegisterCommand(Mesh const& _m, glm::mat4x4 _world, MaterialData const* _mat)
{
	assert(m_hasOpenPass == true && "Can't push command into close pass queue");
	
	DrawCommand d = { _m, _world, _mat };
	m_pendingPasses.back().cmds.push_back(d);
}

void Device::Dispatch(MaterialData const* _mat, uint32_t _groupsX, uint32_t _groupsY, uint32_t _groupsZ)
{
	assert(m_hasOpenPass == true && "Can't push command into close pass queue");
	
	DispatchCommand d = { _mat, {_groupsX, _groupsY, _groupsZ} };
	m_pendingPasses.back().cmds.push_back(d);
}

void Device::EndPass()
{
	assert(m_hasOpenPass == true && "Can't close already closed pass");
	assert(m_pendingPasses.size() > 0 && "Can't close absent pass");
	
	m_hasOpenPass = false;
}

void Device::EndFrame()
{
	for (uint32_t i = 0; i < m_pendingPasses.size(); i++)
	{
		RenderPass& pass = m_pendingPasses[i];
		m_activePass = i;
		
		if (pass.desc.isCompute == false)
		{
			m_boundFB = pass.desc.fb;
			m_viewport = { 0.0f, 0.0f, m_boundFB.width, m_boundFB.height };
			Binning::InitTiles(m_tiles, m_boundFB.width, m_boundFB.height);
			pass.desc.view.renderTargetSize = { m_boundFB.width, m_boundFB.height };
		
			m_ctx.constantBufferHeap.Write<PerPassCB>(m_perPassCB, pass.desc.view);
			
			Clear(m_boundFB, pass.desc.clearFlags);
		}
		
		for (Command const& d : pass.cmds)
			std::visit([this](auto const& _cmd) { ExecCommand(_cmd); }, d);	
	}
	
	m_pendingPasses.clear();
	m_ctx.SetHeapsState(HeapState::FREE);
}

void Device::Present(Handle _texH)
{
	Texture* fb = m_ctx.textureHeap.Get(_texH);
	
	uint32_t width = fb->Width();
	uint32_t height = fb->Height();
	
	bool match = width == WIDTH && height == HEIGHT;
	// assert(width == WIDTH && height == HEIGHT && "Can't present texture not matching final render target format");
	bool isSingleChannel = fb->GetFormat() == Texture::R32F;
	
#pragma omp parallel
#pragma omp for
	for (uint32_t i = 0; i < HEIGHT * WIDTH; i++)
	{
		uint32_t y = i / WIDTH;
		uint32_t x = i % WIDTH;
		
		glm::vec4 v;
		
		if (match)
			v = fb->Load(x, y);
		else
			v = fb->SampleNP({ ((float)x + 0.5f) / (float)WIDTH, ((float)y + 0.5f) / (float)HEIGHT });
		
		if (isSingleChannel)
			v = { v.r, v.r, v.r, 1.0f };	

		m_renderTarget->Store(x, HEIGHT - y - 1, v);
	}
}

void Device::ExecCommand(DrawCommand const& _cmd)
{
	PerObjectCB cb;
	cb.model = _cmd.world;
	cb.normalMatrix = glm::transpose(glm::inverse(cb.model));
	
	m_ctx.constantBufferHeap.Write<PerObjectCB>(m_perObjectCB, cb);
	
	ShaderResources res;
	_cmd.material->Resolve(res);
	res.cbs[CB_PER_PASS] = m_ctx.constantBufferHeap.GetRaw(m_perPassCB);
	res.cbs[CB_LIGHT_PASS] = m_ctx.constantBufferHeap.GetRaw(m_lightCB);
	res.cbs[CB_PER_OBJECT] = m_ctx.constantBufferHeap.GetRaw(m_perObjectCB);
	
	ShaderStages stages = m_ctx.shaderGroupHeap.Get(_cmd.material->PipelineHandle())->Resolve();
	
	std::vector<Vertex_OUT> vBuffer;
	std::vector<size_t> iBuffer;
	
	wPreDiv.clear();
	ShaderBindScope bind(stages, res);
	VertexShaderStep(_cmd.mesh, stages, vBuffer);
	ClippingStep(vBuffer);
	ClipToScreen(vBuffer);
	RasterStep(vBuffer, stages);
}

void Device::ExecCommand(DispatchCommand const& _cmd)
{
	ComputeShader* cs = m_ctx.computeShaderHeap.Get(_cmd.material->PipelineHandle());
	glm::uvec3 gs = cs->GroupSize();
	glm::uvec3 g = _cmd.groups;
	
	DispatchCB cb = { g, 0, g * gs, 0 };
	m_ctx.constantBufferHeap.Write<DispatchCB>(m_dispatchCB, cb);
	
	ShaderResources res;
	_cmd.material->Resolve(res);
	res.cbs[CB_PER_PASS] = m_ctx.constantBufferHeap.GetRaw(m_perPassCB);
	res.cbs[CB_LIGHT_PASS] = m_ctx.constantBufferHeap.GetRaw(m_lightCB);
	res.cbs[CB_PER_OBJECT] = m_ctx.constantBufferHeap.GetRaw(m_dispatchCB);
	
	ComputeBindScope bind(cs, res);
	
	int groupCount = (int)(g.x * g.y * g.z);
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < groupCount; i++)
	{
		glm::uvec3 gId = { i % g.x, (i / g.x) % g.y, i / (g.x * g.y) };
		for (uint32_t z = 0; z < gs.z; z++)
			for (uint32_t y = 0; y < gs.y; y++)
				for (uint32_t x = 0; x < gs.x; x++)
					cs->Execute(gId * gs + glm::uvec3{x, y, z});
	}
}

void Device::SetRenderTarget(Texture* _tex)
{
	m_renderTarget = _tex;
}

void Device::SetLightData(LightData const& _ld)
{
	LightPassCB l;
	FillLightPassCB(_ld, l);
	m_ctx.constantBufferHeap.Write<LightPassCB>(m_lightCB, l);
}

FrameBuffer Device::CreateFrameBuffer(uint32_t _w, uint32_t _h, uint32_t _colorBuffCount, bool _useDepth, vec4 _clearColor)
{
	assert(_colorBuffCount <= MAX_RT && "out of bounds color buffer count");
	
	FrameBuffer out;
	out.useDepth = _useDepth;
	out.width = _w;
	out.height = _h;
	out.clearColor = _clearColor;
	
	for (uint32_t i = 0; i < _colorBuffCount; i++)
		out.gBuffer[i] = m_ctx.textureHeap.Create<Texture>(_w, _h, Texture::RGBA32, Texture::MORTON, Color::WHITE);

	if (_useDepth)
		out.depthBuffer = m_ctx.textureHeap.Create<Texture>(_w, _h, Texture::R32F, Texture::MORTON, Color::BLACK);
	
	return out;
}

FrameBuffer const& Device::GetDefaultFrameBuffer()
{
	return m_defaultFB;
}

void Device::VertexShaderStep(Mesh const& _mesh, ShaderStages _s, std::vector<Vertex_OUT>& _out)
{
	uint32_t vertexCount = _mesh.vertexCount;
	uint32_t indexCount = _mesh.indexCount;
	
	assert(indexCount % 3 == 0 && "Index count must be divisible by 3 : incomplete triangle");
	
	PerPassCB pCB = m_ctx.constantBufferHeap.Read<PerPassCB>(m_perPassCB);
	PerObjectCB oCB = m_ctx.constantBufferHeap.Read<PerObjectCB>(m_perObjectCB);
	
	Vertex const* vBuffer = m_ctx.vertexBufferHeap.GetData(_mesh.vertexBuffer);
	Index const* iBuffer = m_ctx.indexBufferHeap.GetData(_mesh.indexBuffer);

	std::vector<Vertex_OUT> transformed(vertexCount);

	PipelineState state = m_pendingPasses[m_activePass].desc.state;
	
	// Per vertex stage
#pragma omp parallel
#pragma omp for
	for (uint32_t v = 0; v < vertexCount; v++)
	{
		transformed[v] = _s.vs->Execute(vBuffer[v]);
	}

	size_t triCount = indexCount / 3;
	std::vector<uint8_t> visible(triCount);

	// Per triangle stage - backface culling
#pragma omp parallel
#pragma omp for
	for (size_t t = 0; t < triCount; t++)
	{
		Index i0 = iBuffer[3 * t];
		Index i1 = iBuffer[3 * t + 1];
		Index i2 = iBuffer[3 * t + 2];

		assert(i0 < vertexCount && "index out of bounds");
		assert(i1 < vertexCount && "index out of bounds");
		assert(i2 < vertexCount && "index out of bounds");
		
		Triangle currentTriangle = { transformed[i0], transformed[i1], transformed[i2] };
		
		if (state.cullMode == CullMode::NONE)
			visible[t] = true;
		else if (state.cullMode == CullMode::BACK)
			visible[t] = IsFront(currentTriangle);
		else
			visible[t] = !IsFront(currentTriangle);
	}

	// Compaction stage
	_out.clear();
	_out.reserve(indexCount);

	for (size_t t = 0; t < triCount; t++)
	{
		if (visible[t] == false)
			continue;

		Index i0 = iBuffer[3 * t];
		Index i1 = iBuffer[3 * t + 1];
		Index i2 = iBuffer[3 * t + 2];

		_out.push_back(transformed[i0]);
		_out.push_back(transformed[i1]);
		_out.push_back(transformed[i2]);
	}
}

void Device::ClippingStep(std::vector<Vertex_OUT>& _inOut)
{
	std::vector<Vertex_OUT> clippedVertexBuffer;

	for (uint32_t v = 0; v < _inOut.size(); v += 3)
	{
		std::vector<Vertex_OUT> subVBuffer;

		subVBuffer.push_back(_inOut[v]);
		subVBuffer.push_back(_inOut[v + 1]);
		subVBuffer.push_back(_inOut[v + 2]);
		Triangle currentTriangle = { subVBuffer[0], subVBuffer[1], subVBuffer[2] };

		std::vector<uint8_t> frustumBitmasks = Clipping::GetBufferFrustumBitmask(subVBuffer);

		// Edge cases
		uint8_t frustumBitmasksOr = Clipping::GetBufferFrustumBitmaskOr(frustumBitmasks);
		if (frustumBitmasksOr == 0)
		{
			clippedVertexBuffer.insert(clippedVertexBuffer.end(), _inOut.begin() + v, _inOut.begin() + v + 3);
			continue;
		}
		if ((frustumBitmasks[0] & frustumBitmasks[1] & frustumBitmasks[2]) != 0)
			continue;

		
		for (int i = 0; i < 6; i++)
		{
			std::vector<Vertex_OUT> planeSubVB;
			frustumBitmasks = Clipping::GetBufferFrustumBitmask(subVBuffer);
			frustumBitmasksOr = Clipping::GetBufferFrustumBitmaskOr(frustumBitmasks);
			if (((frustumBitmasksOr >> i) & 1) == 0) continue;

			size_t vertexCount = subVBuffer.size();
			for (int j = 0; j < vertexCount; j++)
			{

				bool v1InFrustum = ((frustumBitmasks[j] >> Clipping::FrustumPlane(i)) & 1) == 0;
				bool v2InFrustum = ((frustumBitmasks[(j + 1) % vertexCount] >> Clipping::FrustumPlane(i)) & 1) == 0;

				if (v1InFrustum == false)
				{
					if (v2InFrustum == true)
					{
						float t = Clipping::GetPlaneIntersection(subVBuffer[j], subVBuffer[(j + 1) % vertexCount], Clipping::FrustumPlane(i));
						planeSubVB.push_back(InterpolateVertex(subVBuffer[j], subVBuffer[(j + 1) % vertexCount], t));
					}
				}

				else
				{
					planeSubVB.push_back(subVBuffer[j]);

					if (v2InFrustum == false)
					{
						float t = Clipping::GetPlaneIntersection(subVBuffer[j], subVBuffer[(j + 1) % vertexCount], Clipping::FrustumPlane(i));
						planeSubVB.push_back(InterpolateVertex(subVBuffer[j], subVBuffer[(j + 1) % vertexCount], t));
					}
				}

			}

			subVBuffer = planeSubVB;
		}

		// trianguler
		std::vector<Vertex_OUT> triangulatedSubVBuffer;

		size_t vCount = subVBuffer.size();
		if (vCount < 3) continue;
		for (int i = 0; i < vCount - 2; i++)
		{
			triangulatedSubVBuffer.push_back(subVBuffer[0]);
			triangulatedSubVBuffer.push_back(subVBuffer[i + 1]);
			triangulatedSubVBuffer.push_back(subVBuffer[i + 2]);
		}

		clippedVertexBuffer.insert(clippedVertexBuffer.end(), triangulatedSubVBuffer.begin(), triangulatedSubVBuffer.end());
	}

	_inOut = clippedVertexBuffer;
}

void Device::ClipToScreen(std::vector<Vertex_OUT>& _inOut)
{
	wPreDiv.resize(_inOut.size());
	
#pragma omp parallel for
	for (uint32_t v = 0; v < _inOut.size(); v++)
	{
		wPreDiv[v] = _inOut[v].posH.w;

		// Perspective Divide
		_inOut[v].posH /= _inOut[v].posH.w;

		// Ndc
		_inOut[v].posH = _inOut[v].posH * 0.5f + 0.5f; 

		// To Screen
		_inOut[v].posH[0] = _inOut[v].posH[0] * m_viewport[2] + m_viewport[0];
		_inOut[v].posH[1] = _inOut[v].posH[1] * m_viewport[3] + m_viewport[1];
	}
}

// TODO Add multiple PS output - Output / gBuffer member -> check match

void Device::RasterStep(std::vector<Vertex_OUT> const& _vBuffer, ShaderStages _s)
{
	Binning::Tiles& tiles = m_tiles;
	Binning::ClearTiles(tiles);
	Binning::BlinningStep(_vBuffer, tiles);
	
	PipelineState state = m_pendingPasses[m_activePass].desc.state;
	
	bool writeColor = m_boundFB.gBuffer[0].IsValid();
	
	Texture* fb = writeColor ? m_ctx.textureHeap.Get(m_boundFB.gBuffer[0]) : nullptr;
	Texture* db = m_boundFB.useDepth ? m_ctx.textureHeap.Get(m_boundFB.depthBuffer) : nullptr;
	
	if (m_boundFB.gBuffer[0].IsValid() == false && m_boundFB.useDepth == false)
		return;
	
#pragma omp parallel
#pragma omp for schedule(dynamic)
	for (int i = 0; i < tiles.Count(); i++)
	{
		Binning::Tile const& t = tiles[i];
		
		for (auto const& entry : tiles[i].entries)
		{
			Binning::BoundingBox box = entry.box;
			glm::vec3 baryOffsetX = entry.barOffsetX;
			glm::vec3 baryOffsetY = entry.barOffsetY;
			Index v = entry.index;
			
			Triangle tri = Triangle{ _vBuffer[entry.index], _vBuffer[entry.index + 1], _vBuffer[entry.index + 2] };
			glm::vec3 bary = BarycentricWeight({ (float)t.x + 0.5f, (float)t.y + 0.5f}, tri);
			
			for (int x = box.minX; x < box.maxX; x++)
			{
				for (int y = box.minY; y < box.maxY; y++)
				{
					glm::vec3 baryOffset = (float)(x - t.x) * baryOffsetX + (float)(y - t.y) * baryOffsetY;
					glm::vec3 tBary = bary + baryOffset;
					
					if (IsInTriangle(tBary) == false)
						continue;
					
					glm::vec3 baryP = BarycentricWeightPerspective(tBary, wPreDiv[v], wPreDiv[v + 1], wPreDiv[v + 2]);
					
					glm::vec4 posH = InterpolatePositionH(glm::vec2({ (float)x + 0.5f, (float)y + 0.5f }), tri, tBary);
					float pixelDepth = posH.z;
					
					if (state.depthTest && db)
					{
						if (pixelDepth > 1.0f || pixelDepth < 0.0f)
							continue;
						if (pixelDepth > db->Load(x, y).r)
							continue;	
					}
					
					Vertex_OUT px;
					px.posH = posH;
					InterpolateVertex(px, glm::vec2({ (float)x + 0.5f, (float)y + 0.5f }), tri, baryP);
					
					if (state.depthWrite && db && px.color.a == 1.0f || state.blendMode == Blending::OPAQUE)
						db->Store(x, y, vec4(pixelDepth, 0.0f, 0.0f, 1.0f));
					
					if (writeColor == false)
						continue;
					
					glm::vec4 src = _s.ps->Execute(px);
					if (src.a <= 0.1f)
						continue;
					
					glm::vec4 dest = fb->Load(x, y);
					
					fb->Store(x, y, Blending::Blend(src, dest, state.blendMode)); 
				}
			}
		}
	}
}