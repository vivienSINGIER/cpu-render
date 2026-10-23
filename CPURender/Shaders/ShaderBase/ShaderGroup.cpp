#include "ShaderGroup.h"

#include "../../Core/GraphicsContext.h"

void ShaderGroup::Build()
{
    ShaderStages s = Resolve();
    
    for (int i = 0; i < MAX_CB; i++)
    {
        CBSlotDesc& d = m_cbLayout[i];
        d.isVS = s.vs->m_cb[i].isDeclared;
        d.isPS = s.ps->m_cb[i].isDeclared;
        
        d.isDeclared = d.isVS || d.isPS;
        
        uint16_t vsSize = s.vs->m_cb[i].size;
        uint16_t psSize = s.ps->m_cb[i].size;
        
        if (d.isVS && d.isPS)
            assert(vsSize == psSize && "CB register format don't match accross shader group");
        if (d.isVS)
            d.size = vsSize;
        else if (d.isPS)
            d.size = psSize;
        else
            d.size = 0;
    }

    for (int i = 0; i < MAX_TEX; i++)
    {
        TexSlotDesc& d = m_texLayout[i];
        d.isVS = s.vs->m_tex[i].isDeclared;
        d.isPS = s.ps->m_tex[i].isDeclared;
        d.isDeclared = d.isVS || d.isPS;
        
        if (d.isDeclared == false)
            continue;
        
        bool bothDeclared = d.isVS && d.isPS;
        if (bothDeclared)
            d.isOptional = s.vs->m_tex[i].isOptional && s.ps->m_tex[i].isOptional;
        else if (d.isVS)
            d.isOptional = s.vs->m_tex[i].isOptional;
        else
            d.isOptional = s.ps->m_tex[i].isOptional;
            
    }
}

CBSlotDesc ShaderGroup::CBLayout(int _slot)
{
    assert(_slot < MAX_CB && "Constant buffer slot index out of bounds");
    
    return m_cbLayout[_slot];
}

TexSlotDesc ShaderGroup::TexLayout(int _slot)
{
    assert(_slot < MAX_TEX && "Texture slot index out of bounds");
    
    return m_texLayout[_slot];
}

ShaderStages ShaderGroup::Resolve()
{
    ShaderStages result;
    result.vs = m_ctx->vertexShaderHeap.Get(m_vs);
    result.ps = m_ctx->pixelShaderHeap.Get(m_ps);
    
    assert(result.vs && result.ps && "unable to retrieve shaders");
    
    return result;
}

ShaderGroup::ShaderGroup(GraphicsContext* _ctx, Handle _vs, Handle _ps)
{
    m_ctx = _ctx;
    
    assert(_vs.IsValid() && _ps.IsValid() && "Invalid shader handle");
    
    m_vs = _vs;
    m_ps = _ps;
    Build();
}


