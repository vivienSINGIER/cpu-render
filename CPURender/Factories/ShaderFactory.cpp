#include "ShaderFactory.h"

#include <cassert>
#include <memory>

#include "../Core/GraphicsContext.h"
#include "../Shaders/Shaders.h"

void ShaderFactory::Init(GraphicsContext* _ctx)
{
    assert(g_instance == nullptr && "ShaderFactory already initialized");
    g_instance = new ShaderFactory(_ctx);
    g_instance->RegisterPrebuilts();
}

ShaderFactory* ShaderFactory::Instance()
{
    assert(g_instance != nullptr && "ShaderFactory must be initialized by a GraphicsContext");
    return g_instance;
}

ShaderFactory::ShaderFactory(GraphicsContext* _ctx) : m_ctx(_ctx)
{
}

Handle ShaderFactory::CreateShaderGroup(Handle _vs, Handle _ps)
{
    return m_ctx->shaderGroupHeap.Insert(std::unique_ptr<ShaderGroup>(new ShaderGroup(m_ctx, _vs, _ps)));
}

Material ShaderFactory::CreateMaterial(Handle _shaderGroup)
{
    return Material(new MaterialData(_shaderGroup, m_ctx));
}

Material ShaderFactory::CreateMaterial(std::string const& _libraryName)
{
    return CreateMaterial(GetGroup(_libraryName));
}

void ShaderFactory::RegisterGroup(std::string const& _name, Handle _shaderGroup)
{
    assert(_shaderGroup.IsValid() && "Invalid shader group handle");
    assert(HasGroup(_name) == false && "a shader group is already registered under this name");

    m_library[_name] = _shaderGroup;
}

Handle ShaderFactory::GetGroup(std::string const& _name) const
{
    auto it = m_library.find(_name);
    assert(it != m_library.end() && "no shader group registered under this name");

    return it->second;
}

bool ShaderFactory::HasGroup(std::string const& _name) const
{
    return m_library.find(_name) != m_library.end();
}

void ShaderFactory::RegisterPrebuilts()
{
    RegisterGroup<UnlitC_VS, UnlitC_PS>("UnlitC");
    RegisterGroup<UnlitT_VS, UnlitT_PS>("UnlitT");
    RegisterGroup<LitC_VS, LitC_PS>("LitC");
    RegisterGroup<LitC_BP_VS, LitC_BP_PS>("LitC_BP");
}
