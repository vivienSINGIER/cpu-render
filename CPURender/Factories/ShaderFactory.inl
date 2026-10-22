#ifndef FACTORIES_SHADER_FACTORY_INL_DEFINED
#define FACTORIES_SHADER_FACTORY_INL_DEFINED

#include "ShaderFactory.h"

#include <cassert>
#include <type_traits>

#include "../Core/GraphicsContext.h"

template <typename VS, typename... Args>
Handle ShaderFactory::CreateVertexShader(Args&&... _args)
{
    static_assert(std::is_base_of<VertexShader, VS>::value, "VS must derive from VertexShader");

    return m_ctx->vertexShaderHeap.Create<VS>(m_ctx, std::forward<Args>(_args)...);
}

template <typename PS, typename... Args>
Handle ShaderFactory::CreatePixelShader(Args&&... _args)
{
    static_assert(std::is_base_of<PixelShader, PS>::value, "PS must derive from PixelShader");

    return m_ctx->pixelShaderHeap.Create<PS>(m_ctx, std::forward<Args>(_args)...);
}

template <typename VS, typename PS>
Handle ShaderFactory::CreateShaderGroup()
{
    return CreateShaderGroup(CreateVertexShader<VS>(), CreatePixelShader<PS>());
}

template <typename VS, typename PS>
Handle ShaderFactory::RegisterGroup(std::string const& _name)
{
    Handle h = CreateShaderGroup<VS, PS>();
    RegisterGroup(_name, h);

    return h;
}

#endif
