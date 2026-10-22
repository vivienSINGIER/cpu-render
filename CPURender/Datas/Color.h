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
    static Color YELLOW;
    static Color CYAN;
    static Color MAGENTA;
    static Color ORANGE;
    static Color PURPLE;
    static Color PINK;
    static Color BROWN;
    static Color LIME;
    static Color TEAL;
    static Color NAVY;
    static Color MAROON;
    static Color OLIVE;
    static Color GRAY;
    static Color LIGHT_GRAY;
    static Color DARK_GRAY;
    static Color NONE;
    
    static glm::uint32 ToUint32(Color _c);
    static Color FromUint32(glm::uint32 _uint32);
    
    Color(glm::uint8 _a, glm::uint8 _r, glm::uint8 _g, glm::uint8 _b) : a(_a), r(_r), g(_g), b(_b) {};
    Color() : a(0), r(0), g(0), b(0) {};
    Color operator*(const float &_scal) const { return Color(static_cast<uint8_t>(a * _scal), static_cast<uint8_t>(r * _scal), static_cast<uint8_t>(g * _scal), static_cast<uint8_t>(b * _scal)); }
    Color operator+(const Color &_other) const { return { static_cast<uint8_t>(a+_other.a), static_cast<uint8_t>(r+_other.r), static_cast<uint8_t>(g+_other.g), static_cast<uint8_t>(b+_other.b) }; }
};

#endif