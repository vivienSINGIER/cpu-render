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
    Texture(uint32 _size = 256, Color _color = Color::BLACK);
    ~Texture() = default;

    void Load(std::string const& _filePath);
    void Clear(Color _color = Color::BLACK);
    
    uint32 GetSize() const;
    void SetSize(uint32 _size);
    
    void SetPixelColor(uint32 _x, uint32 _y, Color _color);
    Color GetPixelColor(uint32 _x, uint32 _y);
    
    template <typename T>
    T GetPixelData(uint32 _x, uint32 _y);
    template <typename T>
    void SetPixelData(uint32 _x, uint32 _y, T _data);
    
    Color* GetData();
    
private:
    std::vector<Color> m_data;
    uint32 m_size;
    vec2 m_offsets; 
};

#include "Texture.inl"

#endif
