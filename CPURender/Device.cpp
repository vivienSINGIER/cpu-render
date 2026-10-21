#include "Device.h"

#include "Heaps/Heap.h"
#include "DrawCommand.h"

Device::Device()
{
	//Create the rendertarget resource
}

void Device::Clear()
{
}

void Device::BeginDraw()
{
}

void Device::Draw(Mesh const& _m, glm::mat4x4 _world, Material const& _mat)
{
	DrawCommand d = { _m, _world, _mat };
	m_pendingDraws.push_back(d);
}

void Device::EndDraw()
{
}

void Device::ExecCommand()
{
	for (DrawCommand& d : m_pendingDraws)
	{

	}

	m_pendingDraws.clear();
}

void Device::SetMainCamera(Camera* _c)
{
	m_mainCam = _c;
}
