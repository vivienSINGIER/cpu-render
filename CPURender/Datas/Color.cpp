#include "Color.h"

Color Color::WHITE  = {255, 255, 255, 255};
Color Color::BLACK  = {255, 0, 0, 0};
Color Color::RED    = {255, 255, 0, 0};
Color Color::GREEN  = {255, 0, 255, 0};
Color Color::BLUE   = {255, 0, 0, 255};
Color Color::YELLOW     = {255, 255, 255, 0};
Color Color::CYAN       = {255, 0, 255, 255};
Color Color::MAGENTA    = {255, 255, 0, 255};
Color Color::ORANGE     = {255, 255, 165, 0};
Color Color::PURPLE     = {255, 128, 0, 128};
Color Color::PINK       = {255, 255, 192, 203};
Color Color::BROWN      = {255, 139, 69, 19};
Color Color::LIME       = {255, 50, 205, 50};
Color Color::TEAL       = {255, 0, 128, 128};
Color Color::NAVY       = {255, 0, 0, 128};
Color Color::MAROON     = {255, 128, 0, 0};
Color Color::OLIVE      = {255, 128, 128, 0};
Color Color::GRAY       = {255, 128, 128, 128};
Color Color::LIGHT_GRAY = {255, 192, 192, 192};
Color Color::DARK_GRAY  = {255, 64, 64, 64};
Color Color::NONE   = {0, 0, 0, 0};

glm::uint32 Color::ToUint32(Color _c)
{
    return std::_Bit_cast<glm::uint32>(_c);
}

Color Color::FromUint32(glm::uint32 _uint32)
{
    return std::_Bit_cast<Color>(_uint32);
}

glm::vec4 Color::ToVec4(Color _c)
{
    return glm::vec4((float)_c.a / 255.0f, (float)_c.r / 255.0f, (float)_c.g / 255.0f, (float)_c.b / 255.0f);
}
