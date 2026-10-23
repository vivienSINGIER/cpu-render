#include "Material.h"

#include "../../Core/GraphicsContext.h"

static_assert(MAX_CB <= 32, "m_cbWritten bitmask can't hold more than 32 slots");

void MaterialData::SetTexture(int _slot, Handle _h)
{
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_texLayout[_slot].isDeclared && "cannot bind texture to undeclared slot");
    assert(_h.IsValid() && "Invalid texture handle");

    m_tex[_slot] = _h;
}

void MaterialData::RemoveTexture(int _slot)
{
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_texLayout[_slot].isDeclared && "cannot unbind texture to undeclared slot");
    
    m_tex[_slot] = Handle();
}

bool MaterialData::CheckIntegrity() const
{
    if (m_ctx->shaderGroupHeap.Get(m_shaderGroup) == nullptr)
        return false;

    for (int i = CB_CUSTOM; i < MAX_CB; ++i)
    {
        CBSlotDesc const& l = m_cbLayout[i];
        if (l.isDeclared == false)
            continue;

        if ((m_cbWritten & (1u << i)) == 0)
            return false;
        if (m_cbs[i].Size() != l.size || m_ctx->constantBufferHeap.GetRaw(m_cbs[i]) == nullptr)
            return false;
    }

    for (int i = 0; i < MAX_TEX; ++i)
    {
        if (m_texLayout[i].isDeclared == false)
            continue;
        if (m_texLayout[i].isOptional && m_tex[i].IsValid() == false)
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
}

Handle MaterialData::ShaderGroupHandle() const
{
    return m_shaderGroup;
}

MaterialData::MaterialData(Handle _shaderGroup, GraphicsContext* _ctx)
{
    m_shaderGroup = _shaderGroup;
    m_ctx = _ctx;
    m_cbWritten = 0;

    ShaderGroup* grp = m_ctx->shaderGroupHeap.Get(m_shaderGroup);
    assert(grp && "unable to retrieve shader group");

    for (int i = 0; i < MAX_CB; ++i)
    {
        m_cbLayout[i] = grp->CBLayout(i);
        if (m_cbLayout[i].isDeclared && i >= CB_CUSTOM)
            m_cbs[i] = m_ctx->constantBufferHeap.Create(m_cbLayout[i].size);
    }
    for (int i = 0; i < MAX_TEX; ++i)
        m_texLayout[i] = grp->TexLayout(i);
}

MaterialData::~MaterialData()
{
    for (Handle h : m_cbs)
    {
        if (h.IsValid())
            m_ctx->constantBufferHeap.Delete(h);
    }
}
