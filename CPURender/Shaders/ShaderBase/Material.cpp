#include "Material.h"

#include "../../Core/GraphicsContext.h"

static_assert(MAX_CB <= 32, "m_cbWritten bitmask can't hold more than 32 slots");

void MaterialData::SetTexture(uint32_t _slot, Handle _h)
{
    assert(_slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_layout.tex[_slot].isDeclared && "cannot bind texture to undeclared slot");
    assert(_h.IsValid() && "Invalid texture handle");

    m_tex[_slot] = _h;
}

void MaterialData::RemoveTexture(uint32_t _slot)
{
    assert(_slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_layout.tex[_slot].isDeclared && "cannot unbind texture to undeclared slot");
    
    m_tex[_slot] = Handle();
}

void MaterialData::SetUAVBuffer(uint32_t _slot, Handle _h)
{
    assert(_slot < MAX_UAV && "uav buffer slot index out of bounds");
    assert(m_layout.uav[_slot].isDeclared && "cannot bind uav to undeclared slot");
    assert(_h.IsValid() && "Invalid uav handle");
    assert(m_layout.uav[_slot].type == UAVType::BUFFER && "incompatible uav resource type");
    assert(_h.Stride() == m_layout.uav[_slot].stride && "incompatible uav resource element format");
    
    m_uav[_slot] = _h;
}

void MaterialData::SetRWTexture(uint32_t _slot, Handle _h)
{
    assert(_slot < MAX_UAV && "rw texture slot index out of bounds");
    assert(m_layout.uav[_slot].isDeclared && "cannot bind rw texture to undeclared slot");
    assert(_h.IsValid() && "Invalid rw texture handle");
    assert(m_layout.uav[_slot].type == UAVType::TEXTURE && "incompatible uav resource type");
    assert((_h.HeapId() == TEXTURE_HEAP_ID) && "incompatible handle type");
    
    m_uav[_slot] = _h;
}

bool MaterialData::CheckIntegrity() const
{
    if (m_kind == PipelineKind::GRAPHICS && m_ctx->shaderGroupHeap.Get(m_pipeline) == nullptr)
        return false;
    if (m_kind == PipelineKind::COMPUTE && m_ctx->computeShaderHeap.Get(m_pipeline) == nullptr)
        return false;

    for (int i = CB_CUSTOM; i < MAX_CB; ++i)
    {
        CBSlotDesc const& l = m_layout.cb[i];
        if (l.isDeclared == false)
            continue;

        if ((m_cbWritten & (1u << i)) == 0)
            return false;
        if (m_cbs[i].Stride() != l.size || m_ctx->constantBufferHeap.GetRaw(m_cbs[i]) == nullptr)
            return false;
    }

    for (int i = 0; i < MAX_TEX; ++i)
    {
        if (m_layout.tex[i].isDeclared == false)
            continue;
        if (m_layout.tex[i].isOptional && m_tex[i].IsValid() == false)
            continue;
        
        if (m_ctx->textureHeap.Get(m_tex[i]) == nullptr)
            return false;
    }

    return true;
}

void MaterialData::Resolve(ShaderResources& _out) const
{
    assert(CheckIntegrity() && "Material has unbound or invalid resources");
    
    for (int i = CB_CUSTOM; i < MAX_CB; ++i)
        _out.cbs[i] = m_cbs[i].IsValid() ? m_ctx->constantBufferHeap.GetRaw(m_cbs[i]) : nullptr;
    for (int i = 0; i < MAX_TEX; ++i)
        _out.textures[i] = m_tex[i].IsValid() ? m_ctx->textureHeap.Get(m_tex[i]) : nullptr;
    for (int i = 0; i < MAX_UAV; ++i)
    {
        if (m_layout.uav[i].type == UAVType::BUFFER)
            _out.uavs[i] = m_uav[i].IsValid() ? m_ctx->uavHeap.GetView(m_uav[i]) : UAVView{ nullptr, 0, 0 };
        else
            _out.uavs[i] = m_uav[i].IsValid() ? UAVView{ m_ctx->GetTexture(m_uav[i]), 0, 0 } : UAVView{ nullptr, 0, 0 };
    }
    
    for (int i = 0; i < MAX_UAV; ++i)
    {
        if (m_layout.uav[i].type == UAVType::BUFFER)
            continue;
        
        for (int j = 0; j < MAX_TEX; ++j)
        {
            if (static_cast<Texture*>(_out.uavs[i].data) == _out.textures[j])
                assert(true && "Texture can t be bound in uav and srv");
        }
    }
}

Handle MaterialData::PipelineHandle() const
{
    return m_pipeline;
}

MaterialData::MaterialData(ResourceLayout const& _layout, Handle _pipeline, PipelineKind _kind, GraphicsContext* _ctx)
{
    m_layout = _layout;
    m_pipeline = _pipeline;
    m_ctx = _ctx;
    m_kind = _kind;
    m_cbWritten = 0;
    
    for (int i = 0; i < MAX_CB; i++)
    {
        if (m_layout.cb[i].isDeclared && i >= CB_CUSTOM)
            m_cbs[i] = m_ctx->constantBufferHeap.Create(m_layout.cb[i].size);
    }
}

MaterialData::~MaterialData()
{
    for (Handle h : m_cbs)
    {
        if (h.IsValid())
            m_ctx->constantBufferHeap.Delete(h);
    }
}
