#ifndef FACTORIES_SHADER_FACTORY_H_DEFINED
#define FACTORIES_SHADER_FACTORY_H_DEFINED

#include <string>
#include <unordered_map>

#include "../Heaps/Handle.hpp"
#include "../Shaders/ShaderBase/Shader.h"
#include "../Shaders/ShaderBase/ShaderGroup.h"
#include "../Shaders/ShaderBase/Material.h"

class GraphicsContext;

class ShaderFactory
{
public:
    static ShaderFactory* Instance();
    
    template <typename VS, typename... Args>
    Handle CreateVertexShader(Args&&... _args);
    template <typename PS, typename... Args>
    Handle CreatePixelShader(Args&&... _args);

    Handle CreateShaderGroup(Handle _vs, Handle _ps);
    template <typename VS, typename PS>
    Handle CreateShaderGroup();

    Material CreateMaterial(Handle _shaderGroup);
    Material CreateMaterial(std::string const& _libraryName);
    
    void RegisterGroup(std::string const& _name, Handle _shaderGroup);
    template <typename VS, typename PS>
    Handle RegisterGroup(std::string const& _name);
    
    Handle GetGroup(std::string const& _name) const;
    bool HasGroup(std::string const& _name) const;

private:
    static inline ShaderFactory* g_instance = nullptr;
    GraphicsContext* m_ctx = nullptr;

    std::unordered_map<std::string, Handle> m_library;

    ShaderFactory(GraphicsContext* _ctx);
    static void Init(GraphicsContext* _ctx);

    void RegisterPrebuilts();

    friend class GraphicsContext;
};

#include "ShaderFactory.inl"

#endif
