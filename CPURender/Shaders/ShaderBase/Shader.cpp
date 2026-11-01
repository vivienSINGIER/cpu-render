#include "Shader.h"

#include <cassert>

#include "../../Core/GraphicsContext.h"
#include "../../Datas/ConstantBuffers.hpp"
#include "../../Datas/Lights/LightData.hpp"

void Shader::AddTexture(uint32_t _slot, bool _obtional)
{
    assert(_slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_tex[_slot].isDeclared == false && "can't override texture register declaration");
    
    m_tex[_slot] = { true, _obtional };
}

void Shader::AddRWTexture(uint32_t _slot)
{
    assert(_slot < MAX_UAV && "UAV texture slot index out of bounds");
    assert(m_uavs[_slot].isDeclared == false && "can't override uav register declaration");
    
    m_uavs[_slot] = UAVRegister{ UAVType::TEXTURE, 0, true};
}

Shader::Shader(GraphicsContext* _ctx, uint16_t _cb2Size)
{
    m_ctx = _ctx;

    // reserved slots, AddConstantBuffer refuses them
    m_cb[CB_PER_PASS] = CBRegister{ sizeof(PerPassCB), true };
    m_cb[CB_LIGHT_PASS] = CBRegister{ sizeof(LightPassCB), true };
    m_cb[CB_PER_OBJECT] = CBRegister{ _cb2Size, true };
}

void Shader::Bind(ShaderResources const* _r)
{
    m_r = _r;
}

ResourceLayout Shader::Layout(bool _isVS, bool _isPS)
{
    ResourceLayout l;
    for (int i = 0; i < MAX_CB; i++)
        l.cb[i] = { m_cb[i].size, m_cb[i].isDeclared, _isVS, _isPS };
    for (int i = 0; i < MAX_TEX; i++)
        l.tex[i] = { m_tex[i].isDeclared, m_tex[i].isOptional, _isVS, _isPS };
    for (int i = 0; i < MAX_UAV; i++)
        l.uav[i] = { m_uavs[i].type, m_uavs[i].stride, m_uavs[i].isDeclared };
    return l;
}

bool Shader::HasTexture(uint32_t _slot) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    return m_r->textures[_slot] != nullptr;
}

glm::vec4 Shader::Sample(int _slot, glm::vec2 _uv) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_r->textures[_slot] != nullptr && "Can't access texture data");
    
    Texture const* tex = m_r->textures[_slot];
    
    return tex->SampleNP(_uv);
}

glm::vec4 Shader::SampleLod(int _slot, glm::vec2 _uv, float _lod) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_r->textures[_slot] != nullptr && "Can't access texture data");

    Texture const* tex = m_r->textures[_slot];

    return tex->SampleLod(_uv, _lod);
}

glm::vec4 Shader::SampleGrad(int _slot, glm::vec2 _uv, glm::vec2 _ddx, glm::vec2 _ddy) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_r->textures[_slot] != nullptr && "Can't access texture data");

    Texture const* tex = m_r->textures[_slot];

    return tex->SampleGrad(_uv, _ddx, _ddy);
}

glm::vec4 Shader::SampleCube(int _slot, glm::vec3 _dir) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_r->textures[_slot] != nullptr && "Can't access texture data");

    Texture const* tex = m_r->textures[_slot];

    return tex->SampleNPCube(_dir);
}

glm::vec4 Shader::SampleCubeLod(int _slot, glm::vec3 _dir, float _lod) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_r->textures[_slot] != nullptr && "Can't access texture data");

    Texture const* tex = m_r->textures[_slot];

    return tex->SampleNPCubeLod(_dir, _lod);
}

glm::vec4 Shader::Load(uint32_t _slot, int _x, int _y) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot >= 0 && _slot < MAX_TEX && "Texture slot index out of bounds");
    assert(m_r->textures[_slot] != nullptr && "Can't access texture data");
    
    Texture const* tex = m_r->textures[_slot];
    return tex->Load(_x, _y);
}

glm::vec4 Shader::LoadRW(uint32_t _slot, int _x, int _y) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot < MAX_UAV && "UAV texture slot index out of bounds");
    assert(m_r->uavs[_slot].data != nullptr && "Can't access texture data");
    
    Texture* tex = static_cast<Texture*>(m_r->uavs[_slot].data);
    assert(tex != nullptr && "Can't access UAV texture data");
    
    return tex->Load(_x, _y);
}

void Shader::StoreRW(uint32_t _slot, int _x, int _y, glm::vec4 _color) const
{
    assert(m_r && "shader executed without bound ressources");
    assert(_slot < MAX_UAV && "UAV texture slot index out of bounds");
    assert(m_r->uavs[_slot].data != nullptr && "Can't access texture data");
    
    Texture* tex = static_cast<Texture*>(m_r->uavs[_slot].data);
    assert(tex != nullptr && "Can't access UAV texture data");
    
    tex->Store(_x, _y, _color);
}


PerPassCB const& Shader::Pass() const { return CB<PerPassCB>(CB_PER_PASS); }
LightPassCB const& Shader::Light() const { return CB<LightPassCB>(CB_LIGHT_PASS); }
PerObjectCB const& Shader::Object() const { return CB<PerObjectCB>(CB_PER_OBJECT); }

GraphicsShader::GraphicsShader(GraphicsContext* _ctx) : Shader(_ctx, sizeof(PerObjectCB))
{
}

glm::uvec3 ComputeShader::GroupSize() const
{
    return m_groupSize;
}

ComputeShader::ComputeShader(GraphicsContext* _ctx, glm::uvec3 _groupSize) : Shader(_ctx, sizeof(DispatchCB))
{
    m_groupSize = _groupSize;
}

DispatchCB const& ComputeShader::Dispatch() const { { return CB<DispatchCB>(CB_PER_OBJECT); }}
