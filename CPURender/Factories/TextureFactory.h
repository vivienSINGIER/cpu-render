#ifndef FACTORIES_TEXTURE_FACTORY_H_DEFINED
#define FACTORIES_TEXTURE_FACTORY_H_DEFINED
#include <string>
#include <unordered_map>

#include "../Heaps/Handle.hpp"

class Texture;
class GraphicsContext;

class TextureFactory
{
public:
    static TextureFactory* Instance();
    
    Handle CreateTexture(std::string const& _name, uint32_t _width = 256, uint32_t _height = 256, bool _genMips = false);
    Handle CreateTexture(std::string const& _name, std::string const& _path, bool _isSrgb = false, bool _genMips = true);
    
    Handle GetTextureHandle(std::string const& _name);
    Texture* GetTexture(std::string const& _name);
    Texture* GetTexture(Handle _h);
    
    bool HasTexture(std::string const& _name);
    
private:
    static inline TextureFactory* g_instance = nullptr;    
    GraphicsContext* m_ctx;
    
    std::unordered_map<std::string, Handle> m_library;
    
    TextureFactory(GraphicsContext* _pContext);
    static void Init(GraphicsContext* _pContext);
    
    friend class GraphicsContext;
};

#endif
