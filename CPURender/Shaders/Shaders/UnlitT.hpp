#ifndef SHADERS_UNLIT_T_HPP_DEFINED
#define SHADERS_UNLIT_T_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

class UnlitT_VS : public VertexShader
{
public:
    UnlitT_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}  
    
    Vertex_OUT Execute(ShaderResources const& _r, Vertex const& _in) const override
    {
        PerPassCB const& pCB = CB<PerPassCB>(_r, 0);
        PerObjectCB const& oCB = CB<PerObjectCB>(_r, 1);
        
        Vertex_OUT v;
        v.color = _in.color;
        v.positionModel = oCB.model * glm::vec4({ _in.position, 1 });
        v.positionH = pCB.proj * pCB.view * v.positionModel;
        v.normal = glm::mat3x3(oCB.model) * _in.normal;
        v.uv = _in.uv;
        return v;
    }
};

class UnlitT_PS : public PixelShader
{
public:
    UnlitT_PS(GraphicsContext* _ctx) : PixelShader(_ctx) { AddTexture(0); }
    
    Color Execute(ShaderResources const& _r, Vertex_OUT const& _in) const override
    {
        return Sample(_r, 0, _in.uv );
    }
};

#endif
