#include "Device.h"

#include "../Camera.h"
#include "../main.h"
#include "../Datas/DrawCommand.h"
#include "../Datas/ConstantBuffers.hpp"
#include "../Datas/Triangle.h"
#include "../Clipping.h"
#include "../Shaders/ShaderBase/Material.h"

Device::Device() : m_ctx()
{
	m_viewport = { 0.0f, 0.0f, WIDTH, HEIGHT };
	m_clearColor = Color::BLACK;
	
	m_mainCam = nullptr;
	m_renderTarget = nullptr;
	
	m_frameBuffer = m_ctx.frameBufferHeap.Create<Texture>(WIDTH, HEIGHT, Texture::Encoding::MORTON, Color::WHITE);
	m_depthBuffer = m_ctx.frameBufferHeap.Create<Texture>(WIDTH, HEIGHT, Texture::Encoding::MORTON, Color::BLACK);
	
	m_perPassCB = m_ctx.constantBufferHeap.Create(sizeof(PerPassCB));
	m_perObjectCB = m_ctx.constantBufferHeap.Create(sizeof(PerObjectCB));
}

void Device::Clear()
{
	Texture* rt = m_ctx.frameBufferHeap.Get(m_frameBuffer);
	Texture* depth = m_ctx.frameBufferHeap.Get(m_depthBuffer);
	
	rt->Clear(m_clearColor);
	depth->Clear(std::bit_cast<Color>(1.0f));
}

void Device::BeginDraw()
{
	Clear();
	
	assert(m_mainCam != nullptr && "Can't draw on uninitialized main camera");
	PerPassCB perPassCB;
	perPassCB.deltaTime = 0.066f;
	m_mainCam->FillData(&perPassCB);
	
	m_ctx.constantBufferHeap.Write<PerPassCB>(m_perPassCB, perPassCB);
}

// TODO Add materials
void Device::RegisterCommand(Mesh const& _m, glm::mat4x4 _world, MaterialData const* _mat)
{
	DrawCommand d = { _m, _world, _mat };
	m_pendingDraws.push_back(d);
}

void Device::Draw()
{
	for (DrawCommand& d : m_pendingDraws)
	{
		ExecCommand(d);
	}
	m_pendingDraws.clear();
}

void Device::EndDraw()
{
	assert(m_renderTarget != nullptr && "Can't draw on uninitialized render target");
	
	Texture* fb = m_ctx.frameBufferHeap.Get(m_frameBuffer);
	
#pragma omp parallel
#pragma omp for
	for (uint32_t i = 0; i < HEIGHT * WIDTH; i++)
	{
		uint32_t y = i / WIDTH;
		uint32_t x = i % WIDTH;
		m_renderTarget->SetPixelColor(x, HEIGHT - y - 1, fb->GetPixelColor(x, y));
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
	res.cbs[CB_PER_OBJECT] = m_ctx.constantBufferHeap.GetRaw(m_perObjectCB);
	
	ShaderStages stages = m_ctx.shaderGroupHeap.Get(_cmd.material->ShaderGroupHandle())->Resolve();
	
	std::vector<Vertex_OUT> vBuffer;
	
	VertexShaderStep(_cmd.mesh, res, stages, vBuffer);
	ClippingStep(vBuffer);
	ClipToScreen(vBuffer);
	RasterStep(vBuffer, res, stages);
}

void Device::SetMainCamera(Camera* _c)
{
	m_mainCam = _c;
}

void Device::SetRenderTarget(Texture* _tex)
{
	m_renderTarget = _tex;
}

void Device::VertexShaderStep(Mesh const& _mesh, ShaderResources const& _r, ShaderStages _s, std::vector<Vertex_OUT>& _out)
{
	uint32_t vertexCount = _mesh.vertexCount;
	uint32_t indexCount = _mesh.indexCount;
	
	assert(indexCount % 3 == 0 && "Index count must be divisible by 3 : incomplete triangle");
	
	PerPassCB pCB = m_ctx.constantBufferHeap.Read<PerPassCB>(m_perPassCB);
	PerObjectCB oCB = m_ctx.constantBufferHeap.Read<PerObjectCB>(m_perObjectCB);
	
	Vertex const* vBuffer = m_ctx.vertexBufferHeap.GetData(_mesh.vertexBuffer);
	Index const* iBuffer = m_ctx.indexBufferHeap.GetData(_mesh.indexBuffer);

	std::vector<Vertex_OUT> transformed(vertexCount);

	// Per vertex stage
#pragma omp parallel
#pragma omp for
	for (uint32_t v = 0; v < vertexCount; v++)
	{
		transformed[v] = _s.vs->Execute(_r, vBuffer[v]);
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

		visible[t] = IsFront(currentTriangle);
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

		////clipMask = (IsInVP(currentTriangle.v1->pos) == false) | ((IsInVP(currentTriangle.v2->pos) == false) << 1) | ((IsInVP(currentTriangle.v3->pos) == false) << 2);

		std::array<Vertex_OUT*, 3> triangleVertice;
		triangleVertice[0] = &_inOut[v];
		triangleVertice[1] = &_inOut[v + 1];
		triangleVertice[2] = &_inOut[v + 2];
		Triangle currentTriangle = { *triangleVertice[0], *triangleVertice[1], *triangleVertice[2] };

		std::array<uint8_t, 3> frustumBitmasks;
		frustumBitmasks[0] = Clipping::GetFrustumBitmask(*triangleVertice[0]);
		frustumBitmasks[1] = Clipping::GetFrustumBitmask(*triangleVertice[1]);
		frustumBitmasks[2] = Clipping::GetFrustumBitmask(*triangleVertice[2]);

		// Edge cases
		if ((frustumBitmasks[0] | frustumBitmasks[1] | frustumBitmasks[2]) == 0)
			subVBuffer.insert(subVBuffer.end(), _inOut.begin() + v, _inOut.begin() + v + 3);
		if ((frustumBitmasks[0] & frustumBitmasks[1] & frustumBitmasks[2]) != 0)
			continue;

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				bool v1InFrustum = (frustumBitmasks[j] >> i) & 1;
				bool v2InFrustum = (frustumBitmasks[(j + 1) % 3] >> i) & 1;

				if (v1InFrustum == true)
				{
					if (v2InFrustum == false)
					{
						float t = Clipping::GetPlaneIntersection(*triangleVertice[j], *triangleVertice[(j + 1) % 3], Clipping::FrustumPlane(i));
						glm::vec3 a = glm::vec3(triangleVertice[j]->positionH);
						glm::vec3 b = glm::vec3(triangleVertice[(j + 1) % 3]->positionH);
						subVBuffer.push_back(InterpolateVertex(a + (b - a) * t, currentTriangle));
					}
				}

				else
				{
					subVBuffer.push_back(*triangleVertice[j]);

					if (v2InFrustum == true)
					{
						float t = Clipping::GetPlaneIntersection(*triangleVertice[j], *triangleVertice[(j + 1) % 3], Clipping::FrustumPlane(i));
						glm::vec3 a = glm::vec3(triangleVertice[j]->positionH);
						glm::vec3 b = glm::vec3(triangleVertice[(j + 1) % 3]->positionH);
						subVBuffer.push_back(InterpolateVertex(a + (b-a)*t, currentTriangle));
					}
				}
			}
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
	for (uint32_t v = 0; v < _inOut.size(); v++)
	{
		// Perspective Divide
		_inOut[v].positionH /= _inOut[v].positionH.w;

		// Ndc
		_inOut[v].positionH = _inOut[v].positionH * 0.5f + 0.5f; 

		// To Screen
		_inOut[v].positionH[0] = _inOut[v].positionH[0] * m_viewport[2] + m_viewport[0];
		_inOut[v].positionH[1] = _inOut[v].positionH[1] * m_viewport[3] + m_viewport[1];
	}
}

void Device::RasterStep(std::vector<Vertex_OUT> const& _vBuffer, ShaderResources const& _r, ShaderStages _s)
{
	Texture* fb = m_ctx.frameBufferHeap.Get(m_frameBuffer);
	Texture* db = m_ctx.frameBufferHeap.Get(m_depthBuffer);
	
	// TODO OPEN MP
	for (uint32_t v = 0; v < _vBuffer.size(); v += 3)
	{
		Triangle currentTriangle = { _vBuffer[v], _vBuffer[v + 1], _vBuffer[v + 2] };
		glm::vec4 box = GetBoundingBox(currentTriangle);
		box.x = fmax(box.x, 0);
		box.y = fmax(box.y, 0);
		box.z = fmin(box.z, WIDTH);
		box.w = fmin(box.w, HEIGHT);
		
		for (int32_t x = box.x; x < box.z; x++) 
		{
			for (int32_t y = box.y; y < box.w; y++) 
			{
				if (IsInTriangle({ x + 0.5f, y + 0.5f }, currentTriangle))
				{
					Vertex_OUT px = InterpolateVertex({ x + 0.5f, y + 0.5f }, currentTriangle);
					float pixelDepth = px.positionH.z;
					
					if (pixelDepth > 1.0f || pixelDepth < 0.0f)
						continue;
					if (pixelDepth > db->GetPixelData<float>(x, y))
						continue;

					db->SetPixelData<float>(x, y, px.positionH.z);

					fb->SetPixelColor(x, y, _s.ps->Execute(_r, px));
				}
			}
		}
	}
}