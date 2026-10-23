#ifndef SHADERS_LIT_C_BP_HPP_DEFINED
#define SHADERS_LIT_C_BP_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../Datas/Lights/LightHelpers.hpp"

using namespace Lighting;

class LitC_BP_VS : public VertexShader
{
public:
    LitC_BP_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

    inline Vertex_OUT Execute(ShaderResources const& _r, Vertex const& _in) const override
    {
        PerPassCB const& pCB = CB<PerPassCB>(_r, CB_PER_PASS);
        PerObjectCB const& oCB = CB<PerObjectCB>(_r, CB_PER_OBJECT);

        Vertex_OUT v;
        v.color = _in.color;
        v.positionModel = oCB.model * glm::vec4({ _in.position, 1 });
        v.positionH = pCB.proj * pCB.view * v.positionModel;
        v.normal = glm::mat3x3(oCB.normalMatrix) * _in.normal;
        v.uv = _in.uv;
        return v;
    }
};

class LitC_BP_PS : public PixelShader
{
public:
    struct MaterialCB
    {
        glm::vec4 diffuseAlbedo;
        glm::vec3 fresnelR0;
        float roughness;
    };
    
    LitC_BP_PS(GraphicsContext* _ctx) : PixelShader(_ctx) { AddConstantBuffer<MaterialCB>(CB_CUSTOM); }

    inline Color Execute(ShaderResources const& _r, Vertex_OUT const& _in) const override
    {
        PerPassCB const& pCB = CB<PerPassCB>(_r, CB_PER_PASS);
        LightPassCB const& lCB = CB<LightPassCB>(_r, CB_LIGHT_PASS);
        MaterialCB const& cb = CB<MaterialCB>(_r, CB_CUSTOM);

        glm::vec3 normalW = glm::normalize(_in.normal);
        
        glm::vec3 toEye = glm::normalize(pCB.eyePos - glm::vec3(_in.positionModel));
        glm::vec4 ambient = lCB.ambientLight * cb.diffuseAlbedo;
        
        float shininess = 1.0f - cb.roughness;
        BlinnPhongData d = { cb.diffuseAlbedo, cb.fresnelR0, shininess};
        glm::vec3 shadowFactor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec4 directLight = ComputeLighting(lCB, d, _in.positionModel, normalW, toEye, shadowFactor);
        
        Color litColor = Color::FromVec4(glm::clamp(ambient + directLight, 0.0f, 1.0f));
        litColor.a = (uint8_t)(cb.diffuseAlbedo.a * 255.0f);

        return litColor;
    }
};

#endif
