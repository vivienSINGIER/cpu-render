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
#include "../Datas/Lights/LightHelpers.hpp"
#include "../Shaders/ShaderBase/Material.h"

Device::Device() : m_ctx()
{
	m_renderTarget = nullptr;
	
	m_defaultFB.width = WIDTH;
	m_defaultFB.height = HEIGHT;
	m_defaultFB.gBuffer[0] = m_ctx.textureHeap.Create<Texture>(WIDTH, HEIGHT, Texture::RGBA32, Texture::MORTON, Color::WHITE);
	m_defaultFB.depthBuffer = m_ctx.textureHeap.Create<Texture>(WIDTH, HEIGHT, Texture::R32F, Texture::MORTON, Color::BLACK);
	m_defaultFB.clearColor = vec4(0.f, 0.f, 0.f, 1.0f);
	
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

void Device::RegisterCommand(Mesh const& _m, glm::mat4x4 _world, MaterialData const* _mat, std::vector<glm::mat4x4> _finalBoneMatrices)
{
	assert(m_hasOpenPass == true && "Can't push command into close pass queue");
	
	DrawCommand d = { _m, _world, _mat, _finalBoneMatrices };
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
			pass.desc.passData.renderTargetSize = { m_boundFB.width, m_boundFB.height };
		
			m_ctx.constantBufferHeap.Write<PerPassCB>(m_perPassCB, pass.desc.passData);
			
			Clear(m_boundFB, pass.desc.clearFlags);
		}
		
		for (Command const& d : pass.cmds)
			std::visit([this](auto const& _cmd) { ExecCommand(_cmd); }, d);	
	}
	
	m_ctx.SetHeapsState(HeapState::FREE);
}

void Device::Present(Handle _texH)
{
	Texture* fb = m_ctx.textureHeap.Get(_texH);
	Texture* depth = m_boundFB.useDepth ? m_ctx.textureHeap.Get(m_boundFB.depthBuffer) : nullptr;
	
	uint32_t width = fb->Width();
	uint32_t height = fb->Height();
	
	bool match = width == WIDTH && height == HEIGHT;
	// assert(width == WIDTH && height == HEIGHT && "Can't present texture not matching final render target format");
	bool isSingleChannel = fb->GetFormat() == Texture::R32F;
	
	double logSum = 0.0;
	int32_t count = 0;
	float exposure = m_exposure * 0.18f / m_adaptedLum;
	
#pragma omp parallel
#pragma omp for reduction(+:logSum, count)
	for (int32_t i = 0; i < HEIGHT * WIDTH; i++)
	{
		uint32_t y = i / WIDTH;
		uint32_t x = i % WIDTH;
		
		glm::vec4 v;

		if (match)
			v = fb->Load(x, y);
		else
			v = fb->SampleNP({ ((float)x + 0.5f) / (float)WIDTH, ((float)y + 0.5f) / (float)HEIGHT });

		bool isClear = v == m_boundFB.clearColor; 
			
		vec3 mapped = vec3(v);
		if (isSingleChannel)
			mapped = { v.r, v.r, v.r };
		else if (isClear == false)
		{
			vec3 hdr = vec3(v);
			
			if (v.x != v.x)
				std::cout << "NaN Pixel\n";

			logSum += glm::log(1e-4f + glm::max(Lighting::Luminance(hdr), 0.0f)); // start at small positive value to avoid inf from log on negative number
			count++;
			
			vec3 c = hdr * exposure;
			// float L = Lighting::Luminance(c);
			// mapped = c / (c + 1.0f);
			// mapped = c * ((L / (1.0f + L)) / glm::max(L, 1e-6f)); // map to [0, 1], prevent division by zero on black pixels <- Reinhard approximation
			mapped = glm::clamp((c * (2.51f * c + 0.03f)) / (c * (2.43f * c + 0.59f) + 0.14f), 0.0f, 1.0f); // Narkowicz approximation of the ACES film curve
		}
		
		m_renderTarget->Store(x, HEIGHT - y - 1, vec4(mapped, 1.0f));
	
	}
	
	if (!isSingleChannel && count > 0)
	{
		float avgLum = glm::exp((float)(logSum / count));
		m_adaptedLum += (avgLum - m_adaptedLum) * (1 - exp(-m_pendingPasses[m_activePass].desc.passData.deltaTime)); // TODO use dt
		m_adaptedLum = glm::clamp(m_adaptedLum, 0.01f, 100.0f); // avoid extreme exposure 
	}
	m_pendingPasses.clear();
}

void Device::ExecCommand(DrawCommand const& _cmd)
{
	PerObjectCB cb;
	cb.model = _cmd.world;
	cb.normalMatrix = glm::transpose(glm::inverse(cb.model));

	if (!_cmd.finalBoneMatrices.empty())
	{
		memcpy(cb.finalBonesMatrices, _cmd.finalBoneMatrices.data(), sizeof(glm::mat4x4) * MAX_BONES);
	}

	m_ctx.constantBufferHeap.Write<PerObjectCB>(m_perObjectCB, cb);
	
	ShaderResources res;
	_cmd.material->Resolve(res);
	res.cbs[CB_PER_PASS] = m_ctx.constantBufferHeap.GetRaw(m_perPassCB);
	res.cbs[CB_LIGHT_PASS] = m_ctx.constantBufferHeap.GetRaw(m_lightCB);
	res.cbs[CB_PER_OBJECT] = m_ctx.constantBufferHeap.GetRaw(m_perObjectCB);
	
	ShaderStages stages = m_ctx.shaderGroupHeap.Get(_cmd.material->PipelineHandle())->Resolve();
	
	wPreDiv.clear();
	ShaderBindScope bind(stages, res);
	VertexShaderStep(_cmd.mesh, stages);
	ClippingStep(_cmd.mesh);
	ClipToScreen();
	RasterStep(stages);
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

void Device::VertexShaderStep(Mesh const& _mesh, ShaderStages _s)
{
	uint32_t vertexCount = _mesh.vertexCount;
	uint32_t indexCount = _mesh.indexCount;
	
	assert(indexCount % 3 == 0 && "Index count must be divisible by 3 : incomplete triangle");
	
	Vertex const* vBuffer = m_ctx.vertexBufferHeap.GetData(_mesh.vertexBuffer);
	Index const* iBuffer = m_ctx.indexBufferHeap.GetData(_mesh.indexBuffer);

	if (m_transformed.size() < vertexCount)
		m_transformed.resize(vertexCount);

	PipelineState state = m_pendingPasses[m_activePass].desc.state;
	
	// Per vertex stage
#pragma omp parallel
#pragma omp for schedule(dynamic, 1024)
	for (uint32_t v = 0; v < vertexCount; v++)
	{
		m_transformed[v] = _s.vs->Execute(vBuffer[v]);
	}

	size_t triCount = indexCount / 3;
	if (m_visible.size() < triCount)
		m_visible.resize(triCount);

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
		
		Triangle currentTriangle = { m_transformed[i0], m_transformed[i1], m_transformed[i2] };
		
		if (state.cullMode == CullMode::NONE)
			m_visible[t] = true;
		else if (state.cullMode == CullMode::BACK)
			m_visible[t] = IsFront(currentTriangle);
		else
			m_visible[t] = !IsFront(currentTriangle);
	}
}

// TODO parallelize

void Device::ClippingStep(Mesh const& _mesh)
{
	Index const* iBuffer = m_ctx.indexBufferHeap.GetData(_mesh.indexBuffer);
	size_t triCount = _mesh.indexCount / 3;

	m_clipped.clear();
	m_clipped.reserve(_mesh.indexCount);

	Vertex_OUT polyA[9];
	Vertex_OUT polyB[9];
	
	for (uint32_t t = 0; t < triCount; t++)
	{
		if (m_visible[t] == false)
			continue;
		
		Vertex_OUT const& v0 = m_transformed[iBuffer[3 * t]];
		Vertex_OUT const& v1 = m_transformed[iBuffer[3 * t + 1]];
		Vertex_OUT const& v2 = m_transformed[iBuffer[3 * t + 2]];
		
		uint8 m0 = Clipping::GetFrustumBitmask(v0);
		uint8 m1 = Clipping::GetFrustumBitmask(v1);
		uint8 m2 = Clipping::GetFrustumBitmask(v2);
		
		uint8 planesToClip = m0 | m1 | m2;
		if (planesToClip == 0)
		{
			m_clipped.push_back(v0);
			m_clipped.push_back(v1);
			m_clipped.push_back(v2);
			continue;
		}
		if ((m0 & m1 & m2) != 0)
			continue;

		polyA[0] = v0;
		polyA[1] = v1;
		polyA[2] = v2;
		Vertex_OUT* src = polyA;
		Vertex_OUT* dst = polyB;
		int count = 3;
		
		for (int p = 0; p < 6; p++)
		{
			if ( ((planesToClip >> p) & 1) == 0)
				continue;
			
			count = Clipping::ClipPolygonAgainstPlane(src, count, dst, (Clipping::FrustumPlane)p);
			std::swap(src, dst);
		}
		
		for (int i = 1; i + 1 < count; i++)
		{
			m_clipped.push_back(src[0]);
			m_clipped.push_back(src[i]);
			m_clipped.push_back(src[i + 1]);
		}
	}
}

void Device::ClipToScreen()
{
	wPreDiv.resize(m_clipped.size());
	
#pragma omp parallel for
	for (uint32_t v = 0; v < m_clipped.size(); v++)
	{
		wPreDiv[v] = m_clipped[v].posH.w;

		// Perspective Divide
		m_clipped[v].posH /= m_clipped[v].posH.w;

		// Ndc
		m_clipped[v].posH = m_clipped[v].posH * 0.5f + 0.5f; 

		// To Screen
		m_clipped[v].posH[0] = m_clipped[v].posH[0] * m_viewport[2] + m_viewport[0];
		m_clipped[v].posH[1] = m_clipped[v].posH[1] * m_viewport[3] + m_viewport[1];
	}
}

// TODO Add multiple PS output - Output / gBuffer member -> check match

void Device::RasterStep(ShaderStages _s)
{
	Binning::Tiles& tiles = m_tiles;
	Binning::ClearTiles(tiles);
	Binning::BlinningStep(m_clipped, tiles);
	
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
			
			Triangle tri = Triangle{ m_clipped[entry.index], m_clipped[entry.index + 1], m_clipped[entry.index + 2] };
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

					// screen space uv derivatives for mip selection : uv one pixel to the right and one pixel down
					glm::vec3 baryPX = BarycentricWeightPerspective(tBary + baryOffsetX, wPreDiv[v], wPreDiv[v + 1], wPreDiv[v + 2]);
					glm::vec3 baryPY = BarycentricWeightPerspective(tBary + baryOffsetY, wPreDiv[v], wPreDiv[v + 1], wPreDiv[v + 2]);
					px.uvDdx = InterpolateUV(tri, baryPX) - px.uv;
					px.uvDdy = InterpolateUV(tri, baryPY) - px.uv;
					
					if (state.depthWrite && db && px.color.a >= 1.0f || state.blendMode == Blending::OPAQUE)
						db->Store(x, y, vec4(pixelDepth, 0.0f, 0.0f, 1.0f));
					
					if (writeColor == false)
						continue;
					
					glm::vec4 src = _s.ps->Execute(px);
					if (src.a <= 1e-8f)
						continue;
					
					glm::vec4 dest = fb->Load(x, y);
					
					fb->Store(x, y, Blending::Blend(src, dest, state.blendMode)); 
				}
			}
		}
	}
}