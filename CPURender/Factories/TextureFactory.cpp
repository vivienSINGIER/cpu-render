#include "TextureFactory.h"

#include <cassert>

#include "../Core/GraphicsContext.h"

TextureFactory* TextureFactory::Instance()
{
    assert(g_instance != nullptr && "Texture factory must be initialized by a GraphicsContext");
    return g_instance;
}

Handle TextureFactory::CreateTexture(std::string const& _name, uint32_t _width, uint32_t _height)
{
    assert(HasTexture(_name) == false && "A texture is already initialized under this name");
    Handle h = m_ctx->textureHeap.Create<Texture>(_width, _height);
    m_library[_name] = h;
    return h;
}

Handle TextureFactory::CreateTexture(std::string const& _name, std::string const& _path)
{
    assert(HasTexture(_name) == false && "A texture is already initialized under this name");
    Handle h = m_ctx->textureHeap.Create<Texture>();
    m_library[_name] = h;
    
    GetTexture(h)->Load(_path);
    
    return h;
}

Handle TextureFactory::GetTextureHandle(std::string const& _name)
{
    assert(HasTexture(_name) == true && "No texture is initialized under this name");
    return m_library[_name];
}

Texture* TextureFactory::GetTexture(std::string const& _name)
{
    Handle h = GetTextureHandle(_name);
    assert(h.IsValid() && "Invalid texture handle");
    
    return m_ctx->textureHeap.Get(h);
}

Texture* TextureFactory::GetTexture(Handle _h)
{
    assert(_h.IsValid() && "Invalid texture handle");
    return m_ctx->textureHeap.Get(_h);
}

bool TextureFactory::HasTexture(std::string const& _name)
{
    return m_library.find(_name) != m_library.end();
}


TextureFactory::TextureFactory(GraphicsContext* _pContext) : m_ctx(_pContext)
{
    
}

void TextureFactory::Init(GraphicsContext* _pContext)
{
    assert(g_instance == nullptr && "TextureFactory already initialized");
    g_instance = new TextureFactory(_pContext);
}
