#include "ShaderGroup.h"

#include "../../Core/GraphicsContext.h"

ResourceLayout ShaderGroup::Build()
{
    ShaderStages s = Resolve();
    
    ResourceLayout res;
    
    ResourceLayout vs = s.vs->Layout();
    ResourceLayout ps = s.ps->Layout();
    
    for (int i = 0; i < MAX_CB; i++)
    {
        CBSlotDesc& d = res.cb[i];
        d.isVS = vs.cb[i].isDeclared;
        d.isPS = ps.cb[i].isDeclared;
        
        d.isDeclared = d.isVS || d.isPS;
        
        uint16_t vsSize = vs.cb[i].size;
        uint16_t psSize = ps.cb[i].size;
        
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
        TexSlotDesc& d = res.tex[i];
        d.isVS = vs.tex[i].isDeclared;
        d.isPS = ps.tex[i].isDeclared;
        d.isDeclared = d.isVS || d.isPS;
        
        if (d.isDeclared == false)
            continue;
        
        bool bothDeclared = d.isVS && d.isPS;
        if (bothDeclared)
            d.isOptional = vs.tex[i].isOptional && ps.tex[i].isOptional;
        else if (d.isVS)
            d.isOptional = vs.tex[i].isOptional;
        else
            d.isOptional = ps.tex[i].isOptional;
            
    }
    
    for (int i = 0; i < MAX_UAV; i++)
    {
        UAVSlotDesc& d = res.uav[i];
        
        bool eqDec = vs.uav[i].isDeclared == ps.uav[i].isDeclared;
        bool eqType = vs.uav[i].type == ps.uav[i].type;
        bool eqStride = vs.uav[i].stride == ps.uav[i].stride;
        
        assert(eqDec && eqType && eqStride && "Uav register don't match between group members");
        
        d = vs.uav[i];
    }
    
    return res;
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


