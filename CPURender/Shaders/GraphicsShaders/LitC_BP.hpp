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

    inline Vertex_OUT Execute(Vertex const& _in) const override
    {
        Vertex_OUT v;
        v.color = _in.color;
        v.posModel = Object().model * glm::vec4({ _in.position, 1 });
        v.posH = Pass().proj * Pass().view * v.posModel;
        v.normal = glm::mat3x3(Object().normalMatrix) * _in.normal;
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
        
        float metalness;
        float transparency;
        bool hasTransparency;
    };
    
    SHADER_CB(MaterialCB, mat, CB_CUSTOM)

    LitC_BP_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    inline glm::vec4 Execute(Vertex_OUT const& _in) const override
    {
        glm::vec3 normalW = glm::normalize(_in.normal);

        glm::vec3 toEye = glm::normalize(Pass().eyePos - glm::vec3(_in.posModel));
        glm::vec4 ambient = Light().ambientLight * mat().diffuseAlbedo;

        float shininess = 1.0f - mat().roughness;
        glm::vec4 diffuse = mat().diffuseAlbedo * (1.0f - mat().metalness);
        BlinnPhongData d = { diffuse, mat().fresnelR0, shininess};
        float shadowFactor = 0.0f;
        glm::vec4 directLight = ComputeLighting(Light(), d, _in.posModel, normalW, toEye, shadowFactor);

        vec4 litColor = clamp(ambient + directLight, 0.0f, 1.0f);
        litColor.a = mat().diffuseAlbedo.a * (1.0f - mat().transparency);

        return litColor;
    }
};

#endif
