#ifndef SHADERS_UNLIT_C_HPP_DEFINED
#define SHADERS_UNLIT_C_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

class UnlitVS : public VertexShader
{
public:
    UnlitVS(GraphicsContext* _ctx) : VertexShader(_ctx) {}  
    
    Vertex_OUT Execute(ShaderResources const& _r, Vertex const& _in) const override
    {
        PerPassCB const& pCB = CB<PerPassCB>(_r, 0);
        PerObjectCB const& oCB = CB<PerObjectCB>(_r, 1);
        
        Vertex_OUT v;
        v.color = _in.color;
        // v.position = glm::project(_in.position, pCB.view * oCB.model, pCB.proj, {0.0f, 0.0f, pCB.renderTargetSize});
        return v;
    }
};

#endif
