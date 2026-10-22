#ifndef TEXTURE_H_DEFINED
#define TEXTURE_H_DEFINED

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/ext/scalar_uint_sized.hpp>

#include "Color.h"

using namespace glm;

class Texture
{
public:
    enum Encoding { MORTON, LINEAR };
    
    Texture(uint32 _width = 256, uint32 _height = 256, Encoding _enc = MORTON, Color _color = Color::BLACK);
    ~Texture() = default;

    Texture ToLinear() const;
    
    void Load(std::string const& _filePath);
    void Clear(Color _color = Color::BLACK);
    
    uint32 Size()                                   const;
    uint32 Width()                                  const;
    uint32 Height()                                 const;
    uint32 GetSize(uint32& _width, uint32& _height) const;
    void SetSize(uint32 _width, uint32 _height);
    
    Color SampleNP(glm::vec2 uv) const;
    
    void SetPixelColor(uint32 _x, uint32 _y, Color _color);
    Color GetPixelColor(uint32 _x, uint32 _y)  const;
    
    template <typename T>
    T GetPixelData(uint32 _x, uint32 _y);
    template <typename T>
    void SetPixelData(uint32 _x, uint32 _y, T _data);
    
    Color* GetData();
    
private:
    std::vector<Color> m_data;
    uint32 m_width;
    uint32 m_height;
    uint32 m_size;
    
    Encoding m_encoding;
    
    uint32 GetIndex(uint32 _x, uint32 _y) const;
};

#include "Texture.inl"

#endif
