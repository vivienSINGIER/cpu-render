#ifndef SHADERS_SHADER_BASE_SHADER_H_DEFINED
#define SHADERS_SHADER_BASE_SHADER_H_DEFINED

#include <array>
#include <glm/vec2.hpp>

#include "../../Heaps/Handle.hpp"
#include "../../Datas/Color.h"
#include "../../Datas/Mesh.h"
#include "../../Datas/ConstantBuffers.hpp"
#include "../../Datas/Lights/LightData.hpp"

#include "ResourceLayout.hpp"

class Texture;
class GraphicsContext;

class Shader
{
public:
    template <typename T>
    void AddConstantBuffer(uint32_t _slot);
    void AddTexture(uint32_t _slot, bool _optional = false);
    
    template <typename T>
    void AddUAV(uint32_t _slot);
    void AddRWTexture(uint32_t _slot);

    virtual ~Shader() = default;

protected:
    Shader(GraphicsContext* _ctx, uint16_t _cb2Size);

    template <typename T>
    T const& CB(uint32_t _slot) const;
    
    template <typename T>
    T const& Load(uint32_t _slot, int _index) const;
    template <typename T>
    void Store(uint32_t _slot, int _index, T const& _value) const;
    
    PerPassCB const& Pass() const;
    LightPassCB const& Light() const;
    PerObjectCB const& Object() const;

    bool HasTexture(uint32_t _slot) const;
    glm::vec4 Sample(int _slot, glm::vec2 _uv) const;
    glm::vec4 SampleCube(int _slot, glm::vec3 _dir) const;
    glm::vec4 Load(uint32_t _slot, int _x, int _y) const;
    
    glm::vec4 LoadRW(uint32_t _slot, int _x, int _y) const;
    void StoreRW(uint32_t _slot, int _x, int _y, glm::vec4 _color) const;

private:
    std::array<CBRegister, MAX_CB> m_cb;
    std::array<TexRegister, MAX_TEX> m_tex;
    std::array<UAVRegister, MAX_UAV> m_uavs;

    GraphicsContext* m_ctx;
    ShaderResources const* m_r = nullptr;
    
    void Bind(ShaderResources const* _r);
    
    ResourceLayout Layout(bool _isVS = false, bool _isPS = false);

    friend class Device;
    friend class ShaderFactory;
    friend class ShaderGroup;
    friend struct ShaderBindScope;
    friend struct ComputeBindScope;
};

#include "Shader.inl"

class GraphicsShader : public Shader
{
protected: 
    using Shader::Shader;
    GraphicsShader(GraphicsContext* _ctx);
};

class VertexShader : public GraphicsShader { public: virtual Vertex_OUT Execute(Vertex const& _in) const = 0; protected: using GraphicsShader::GraphicsShader; };

class PixelShader  : public GraphicsShader { public: virtual glm::vec4 Execute(Vertex_OUT const& _in) const = 0; protected: using GraphicsShader::GraphicsShader; };

class ComputeShader : public Shader
{
public: 
    virtual void Execute(glm::uvec3 _threadId) const = 0; 
    
    glm::uvec3 GroupSize() const;
    
protected: 
    ComputeShader(GraphicsContext* _ctx, glm::uvec3 _groupSize);
    
    DispatchCB const& Dispatch() const;
    
private:
    glm::uvec3 m_groupSize;
    
    using Shader::Object;
};

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

struct ComputeBindScope
{
    ComputeShader* cs;
    ComputeBindScope(ComputeShader* _s, ShaderResources const& _r) : cs(_s) { cs->Bind(&_r); }
    ~ComputeBindScope() { cs->Bind(nullptr); }
};

#define SHADER_CB(Type, name, slot)                                         \
    private: int m_bind_##name = (AddConstantBuffer<Type>(slot), slot);     \
    public: Type const& name() const { return CB<Type>(slot); }             \
            static constexpr int CB_SLOT_##name = slot;

#define SHADER_TEX(name, slot)                                              \
    private: int m_bind_##name = (AddTexture(slot), slot);                  \
    public: glm::vec4 name(glm::vec2 _uv) const { return Sample(slot, _uv); }   \
            glm::vec4 name(uint32_t _x, uint32_t _y) const                      \
            { return Load(slot, _x, _y); }                                  \
            static constexpr int TEX_SLOT_##name = slot;

#define SHADER_TEX_OPT(name, slot)                                          \
    private: int m_bind_##name = (AddTexture(slot, true), slot);            \
    public: glm::vec4 name(glm::vec2 _uv) const { return Sample(slot, _uv); }   \
            glm::vec4 Sample_Cube_##name(glm::vec3 _dir) const { return SampleCube(slot, _dir); }   \
            glm::vec4 name(uint32_t _x, uint32_t _y) const                      \
            { return Load(slot, _x, _y); }                                  \
            bool Has_##name() const { return HasTexture(slot); }            \
            static constexpr int TEX_SLOT_##name = slot;

#define SHADER_UAV_BUFF(Type, name, slot)                                   \
    private: int m_bind_##name = (AddUAV<Type>(slot), slot);                \
    public: Type const& Load_##name(int _index) const                       \
            { return Load<Type>(slot, _index); }                            \
            void Store_##name(int _index, Type const& _value)  const        \
            { Store<Type>(slot, _index, _value); }                          \
            static constexpr int UAV_SLOT_##name = slot;

#define SHADER_UAV_TEX(name, slot)                                          \
    private: int m_bind_##name = (AddRWTexture(slot), slot);                \
    public: glm::vec4 Load_##name(int _x, int _y) const                         \
            { return LoadRW(slot, _x, _y); }                                \
            void Store_##name(int _x, int _y, glm::vec4 _color) const           \
            { StoreRW(slot, _x, _y, _color); }                          \
            static constexpr int UAV_SLOT_##name = slot;

#endif
