#include "Device.h"

#include "../main.h"
#include "../Datas/DrawCommand.h"
#include "../Datas/ConstantBuffers.hpp"

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
	depth->Clear();
}

void Device::BeginDraw()
{
}

void Device::RegisterCommand(Mesh const& _m, glm::mat4x4 _world, Material const& _mat)
{
	DrawCommand d = { _m, _world };
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
		uint32_t y = i / HEIGHT;
		uint32_t x = i % HEIGHT;
		m_renderTarget->SetPixelColor(x, y, fb->GetPixelColor(x, y));
	}
}

void Device::ExecCommand(DrawCommand const& _cmd)
{
	
}

void Device::SetMainCamera(Camera* _c)
{
	m_mainCam = _c;
}

void Device::SetRenderTarget(Texture* _tex)
{
	m_renderTarget = _tex;
}
