#ifndef SHADER_BASE_MATERIAL_H_DEFINED
#define SHADER_BASE_MATERIAL_H_DEFINED

#include "../../Heaps/Handle.hpp"
#include "Shader.h"
#include "ShaderGroup.h"

class GraphicsContext;

class MaterialData
{
public:
    ~MaterialData();
    MaterialData(MaterialData const&) = delete;
    MaterialData& operator=(MaterialData const&) = delete;

    template <typename T>
    void SetParams(int _slot, T const& _value);
    void SetTexture(int _slot, Handle _h);
    void RemoveTexture(int _slot);
    
    bool CheckIntegrity() const;
    
    void Resolve(ShaderResources& _out) const;
    Handle ShaderGroupHandle() const;
    
private:
    Handle m_shaderGroup;
    std::array<Handle, MAX_CB> m_cbs;
    std::array<Handle, MAX_TEX> m_tex;
    
    std::array<CBSlotDesc, MAX_CB> m_cbLayout;
    std::array<TexSlotDesc, MAX_TEX> m_texLayout;

    uint32_t m_cbWritten;

    GraphicsContext* m_ctx;
    
    MaterialData(Handle _shaderGroup, GraphicsContext* _ctx);

    friend class ShaderFactory;
};

using Material = std::unique_ptr<MaterialData>;

#include "Material.inl"

#endif

