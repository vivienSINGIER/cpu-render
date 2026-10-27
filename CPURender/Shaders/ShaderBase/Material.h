#ifndef SHADER_BASE_MATERIAL_H_DEFINED
#define SHADER_BASE_MATERIAL_H_DEFINED

#include <memory>

#include "../../Heaps/Handle.hpp"
#include "Shader.h"
#include "ShaderGroup.h"

enum class PipelineKind { GRAPHICS, COMPUTE };

class GraphicsContext;

class MaterialData
{
public:
    ~MaterialData();
    MaterialData(MaterialData const&) = delete;
    MaterialData& operator=(MaterialData const&) = delete;

    template <typename T>
    void SetParams(uint32_t _slot, T const& _value);
    
    void SetTexture(uint32_t _slot, Handle _h);
    void RemoveTexture(uint32_t _slot);
    
    void SetUAVBuffer(uint32_t _slot, Handle _h);
    void SetRWTexture(uint32_t _slot, Handle _h);
    
    bool CheckIntegrity() const;
    
    void Resolve(ShaderResources& _out) const;
    Handle PipelineHandle() const;
    
private:
    PipelineKind m_kind;
    Handle m_pipeline;
    ResourceLayout m_layout;
    
    std::array<Handle, MAX_CB> m_cbs;
    std::array<Handle, MAX_TEX> m_tex;
    std::array<Handle, MAX_UAV> m_uav;
    
    uint32_t m_cbWritten;
    GraphicsContext* m_ctx;
    
    MaterialData(ResourceLayout const& _layout, Handle _pipeline, PipelineKind _kind, GraphicsContext* _ctx);

    friend class ShaderFactory;
};

using Material = std::unique_ptr<MaterialData>;

#include "Material.inl"

#endif

