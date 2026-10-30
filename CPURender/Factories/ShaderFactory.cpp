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
    assert(_shaderGroup.HeapId() == SHADER_GROUP_HEAP_ID && "invalid shader group handle");
    ShaderGroup* g = m_ctx->shaderGroupHeap.Get(_shaderGroup);
    assert(g && "unable to retrieve shader group");
    return Material(new MaterialData(g->Build(), _shaderGroup, PipelineKind::GRAPHICS, m_ctx));
}

Material ShaderFactory::CreateMaterial(std::string const& _libraryName)
{
    return CreateMaterial(GetGroup(_libraryName));
}

Material ShaderFactory::CreateComputeMaterial(Handle _computeShader)
{
    assert(_computeShader.HeapId() == COMPUTE_SHADER_HEAP_ID && "invalid compute shader handle");
    ComputeShader* cs = m_ctx->computeShaderHeap.Get(_computeShader);
    assert(cs && "unable to retrieve compute shader");
    return Material(new MaterialData(cs->Layout(), _computeShader, PipelineKind::COMPUTE, m_ctx));
}

Material ShaderFactory::CreateComputeMaterial(std::string const& _libraryName)
{
    return CreateComputeMaterial(GetCompute(_libraryName));
}

void ShaderFactory::RegisterGroup(std::string const& _name, Handle _shaderGroup)
{
    assert(_shaderGroup.IsValid() && "Invalid shader group handle");
    assert(_shaderGroup.HeapId() == SHADER_GROUP_HEAP_ID && "invalid shader group handle");
    assert(HasGroup(_name) == false && "a pipeline is already registered under this name");

    m_library[_name] = _shaderGroup;
}

void ShaderFactory::RegisterCompute(std::string const& _name, Handle _computeHandle)
{
    assert(_computeHandle.IsValid() && "Invalid compute shader handle");
    assert(_computeHandle.HeapId() == COMPUTE_SHADER_HEAP_ID && "invalid compute shader handle");
    assert(HasGroup(_name) == false && "a pipeline is already registered under this name");

    m_library[_name] = _computeHandle;
}

Handle ShaderFactory::GetGroup(std::string const& _name) const
{
    auto it = m_library.find(_name);
    assert(it != m_library.end() && "no shader group registered under this name");

    assert(it->second.HeapId() == SHADER_GROUP_HEAP_ID && "invalid shader group handle");
    return it->second;
}

Handle ShaderFactory::GetCompute(std::string const& _name) const
{
    auto it = m_library.find(_name);
    assert(it != m_library.end() && "no compute shader registered under this name");

    assert(it->second.HeapId() == COMPUTE_SHADER_HEAP_ID && "invalid compute shader handle");
    return it->second;
}

bool ShaderFactory::HasGroup(std::string const& _name) const
{
    return m_library.find(_name) != m_library.end();
}

bool ShaderFactory::HasCompute(std::string const& _name) const
{
    return HasGroup(_name);
}

void ShaderFactory::RegisterPrebuilts()
{
    RegisterGroup<UnlitC_VS, UnlitC_PS>("UnlitC");
    RegisterGroup<UnlitT_VS, UnlitT_PS>("UnlitT");
    RegisterGroup<UnlitSkybox_VS, UnlitSkybox_PS>("UnlitSkybox");
    RegisterGroup<LitC_VS, LitC_PS>("LitC");
    RegisterGroup<LitC_BP_VS, LitC_BP_PS>("LitC_BP");
    RegisterGroup<LitT_BP_VS, LitT_BP_PS>("LitT_BP");
    RegisterGroup<LitC_BP_Cube_VS, LitC_BP_Cube_PS>("LitC_BP_Cube");

    RegisterGroup<LitC_BP_Transparent_VS, LitC_BP_Transparent_PS>("LitC_BP_Transparent");
    
    RegisterGroup<PBR_VS, PBR_PS>("PBR");
    RegisterGroup<LitT_BP_Cube_VS, LitT_BP_Cube_PS>("LitT_BP_Cube");

    RegisterCompute<FB_Fusion_CS>("FBFusion");
    RegisterCompute<GroupDebug_CS>("GroupDebug");
    RegisterCompute<GammaCorr_CS>("GammaCorrect");
    RegisterCompute<TexCopy_CS>("TexCopy");
}
