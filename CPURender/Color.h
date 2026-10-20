#ifndef COLOR_H_DEFINED
#define COLOR_H_DEFINED

#include <glm/ext/scalar_uint_sized.hpp>
#include <bit>

struct Color
{
    glm::uint8 a, r, g, b;
    
    static Color WHITE;
    static Color BLACK;
    static Color RED;
    static Color GREEN;
    static Color BLUE;
    
    static glm::uint32 ToUint32(Color _c);
    static Color FromUint32(glm::uint32 _uint32);
};

#endif