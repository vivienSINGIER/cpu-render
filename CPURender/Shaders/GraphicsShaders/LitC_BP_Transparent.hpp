#ifndef SHADERS_LIT_C_BP_TRANSPARENT_HPP_DEFINED
#define SHADERS_LIT_C_BP_TRANSPARENT_HPP_DEFINED

#include <glm/ext/matrix_projection.hpp>

#include "../../Datas/ConstantBuffers.hpp"
#include "../ShaderBase/Shader.h"

#include "../../Datas/Lights/LightHelpers.hpp"

using namespace Lighting;

class LitC_BP_Transparent_VS : public VertexShader
{
public:
    LitC_BP_Transparent_VS(GraphicsContext* _ctx) : VertexShader(_ctx) {}

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

class LitC_BP_Transparent_PS : public PixelShader
{
public:
    struct MaterialCB
    {
        glm::vec4 diffuseAlbedo;

        glm::vec3 fresnelR0;
        float roughness;

        float metalness;
        float transparency;
        float refractionFactor;
        bool hasTransparency;

        glm::vec2 resolution;
    };

    SHADER_CB(MaterialCB, mat, CB_CUSTOM)
        SHADER_TEX_OPT(framebufferTex, 0)
        SHADER_TEX_OPT(framebufferDepth, 1)
        SHADER_TEX_OPT(cubemap, 2)

        LitC_BP_Transparent_PS(GraphicsContext* _ctx) : PixelShader(_ctx) {}

    inline glm::vec4 Execute(Vertex_OUT const& _in) const override
    {
        glm::vec3 normalW = glm::normalize(_in.normal);
        
        glm::vec3 toEye = glm::normalize(Pass().eyePos - glm::vec3(_in.posModel));
        glm::vec4 ambient = Light().ambientLight * mat().diffuseAlbedo;

        float shininess = 1.0f - mat().roughness;
        glm::vec4 diffuse = mat().diffuseAlbedo * (1.0f - mat().metalness);
        BlinnPhongData d = { diffuse, mat().fresnelR0, shininess };
        float shadowFactor = 0.0f;
        glm::vec4 directLight = ComputeLighting(Light(), d, _in.posModel, normalW, toEye, shadowFactor);

        glm::vec2 uv = glm::vec2(_in.posH.x, _in.posH.y) / mat().resolution;
        glm::vec3 dir = glm::refract(-toEye, normalW, 1.0f / mat().refractionFactor);
        dir = glm::mat3(Pass().view) * (dir + toEye);
        glm::vec2 displacedUv = uv + glm::vec2(dir);

        glm::vec4 litColor = ambient + directLight;
        litColor.a = mat().transparency;

        if (displacedUv.x < 0.0f || displacedUv.x > 1.0f || displacedUv.y < 0.0f || displacedUv.y > 1.0f) // if out of bounds, don't refract
        {
            litColor = glm::mix(litColor, framebufferTex(uv), mat().transparency);
            //litColor = { 1, 0, 1, 1 };
        }

        else if (framebufferDepth(displacedUv).x < _in.posH.z)
        {
            litColor = glm::mix(litColor, framebufferTex(uv), mat().transparency);
            //litColor = { 1, 0, 1, 1 };
        }

        else 
        {
            litColor = glm::mix(litColor, framebufferTex(displacedUv), mat().transparency);
            litColor.a = 1;
        }

        return litColor;
    }
};

#endif
