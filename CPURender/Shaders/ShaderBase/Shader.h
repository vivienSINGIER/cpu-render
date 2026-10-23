#ifndef SHADERS_SHADER_BASE_SHADER_H_DEFINED
#define SHADERS_SHADER_BASE_SHADER_H_DEFINED

#include <array>
#include <glm/vec2.hpp>

#include "../../Heaps/Handle.hpp"
#include "../../Datas/Color.h"
#include "../../Datas/Mesh.h"
#include "../../Datas/ConstantBuffers.hpp"
#include "../../Datas/Lights/LightData.hpp"

#define MAX_CB 8
#define MAX_TEX 8

// registers declared by every shader
#define CB_PER_PASS 0
#define CB_LIGHT_PASS 1
#define CB_PER_OBJECT 2
#define CB_CUSTOM 3

class Texture;
class GraphicsContext;

struct CBRegister { uint16_t size; bool isDeclared; };
struct TexRegister { bool isDeclared; bool isOptional; };

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
    void AddTexture(int _slot, bool _optional = false);

    virtual ~Shader() = default;

protected:
    Shader(GraphicsContext* _ctx);

    template <typename T>
    T const& CB(int _slot) const;
    
    PerPassCB const& Pass() const;
    LightPassCB const& Light() const;
    PerObjectCB const& Object() const;

    bool HasTexture(int _slot) const;
    Color Sample(int _slot, glm::vec2 _uv) const;

private:
    std::array<CBRegister, MAX_CB> m_cb;
    std::array<TexRegister, MAX_TEX> m_tex;

    GraphicsContext* m_ctx;
    ShaderResources const* m_r = nullptr;
    
    void Bind(ShaderResources const* _r);

    friend class Device;
    friend class ShaderFactory;
    friend class ShaderGroup;
    friend struct ShaderBindScope;
};

#include "Shader.inl"

class VertexShader : public Shader { public: virtual Vertex_OUT Execute(Vertex const& _in) const = 0; protected: using Shader::Shader; };

class PixelShader  : public Shader { public: virtual Color Execute(Vertex_OUT const& _in) const = 0; protected: using Shader::Shader; };


struct ShaderStages
{
    VertexShader* vs;
    PixelShader* ps;
};

struct ShaderBindScope
{
    ShaderStages s;
    ShaderBindScope(ShaderStages _s, ShaderResources const& _r) : s(_s) { s.vs->Bind(&_r); s.ps->Bind(&_r); }
    ~ShaderBindScope() { s.vs->Bind(nullptr); s.ps->Bind(nullptr); }
};

#define SHADER_CB(Type, name, slot)                                         \
    private: int m_bind_##name = (AddConstantBuffer<Type>(slot), slot);     \
    public: Type const& name() const { return CB<Type>(slot); }             \
            static constexpr int CB_SLOT_##name = slot;

#define SHADER_TEX(name, slot)                                              \
    private: int m_bind_##name = (AddTexture(slot), slot);                  \
    public: Color name(glm::vec2 _uv) const { return Sample(slot, _uv); }   \
            static constexpr int TEX_SLOT_##name = slot;

#define SHADER_TEX_OPT(name, slot)                                              \
    private: int m_bind_##name = (AddTexture(slot, true), slot);                \
    public: Color name(glm::vec2 _uv) const { return Sample(slot, _uv); }       \
            bool Has_##name() const { return HasTexture(slot); }                \
            static constexpr int TEX_SLOT_##name = slot;

#endif
