#ifndef SHADERS_SHADER_BASE_SHADER_H_DEFINED
#define SHADERS_SHADER_BASE_SHADER_H_DEFINED

#include <array>
#include <glm/vec2.hpp>

#include "../../Heaps/Handle.hpp"
#include "../../Datas/Color.h"
#include "../../Datas/Mesh.h"

#define MAX_CB 8
#define MAX_TEX 8

// registers declared by every shader
#define CB_PER_PASS 0
#define CB_PER_OBJECT 1
#define CB_CUSTOM 2

class Texture;
class GraphicsContext;

struct CBRegister { uint16_t size; bool isDeclared; };
struct TexRegister { bool isDeclared; };

struct ShaderResources
{
    std::array<uint8_t const*, MAX_CB> cbs{};
    std::array<Texture const*, MAX_TEX> textures{};
};

class Shader
{
public:
    template <typename T>
    void AddConstantBuffer(int _slot);
    void AddTexture(int _slot);

    virtual ~Shader() = default;

protected:
    Shader(GraphicsContext* _ctx);

    template <typename T>
    T const& CB(ShaderResources const& _r, int _slot) const;
    Color Sample(ShaderResources const& _r, int _slot, glm::vec2 _uv) const;

private:
    std::array<CBRegister, MAX_CB> m_cb;
    std::array<TexRegister, MAX_TEX> m_tex;

    GraphicsContext* m_ctx;

    friend class ShaderFactory;
    friend class ShaderGroup;
};

#include "Shader.inl"

class VertexShader : public Shader { public: virtual Vertex_OUT Execute(ShaderResources const& _r, Vertex const& _in) const = 0; protected: using Shader::Shader; };

class PixelShader  : public Shader { public: virtual Color Execute(ShaderResources const& _r, Vertex_OUT const& _in) const = 0; protected: using Shader::Shader; };

#endif
