#include "Color.h"

Color Color::WHITE  = {255, 255, 255, 255};
Color Color::BLACK  = {255, 0, 0, 0};
Color Color::RED    = {255, 255, 0, 0};
Color Color::GREEN  = {255, 0, 255, 0};
Color Color::BLUE   = {255, 0, 0, 255};

glm::uint32 Color::ToUint32(Color _c)
{
    return std::_Bit_cast<glm::uint32>(_c);
}

Color Color::FromUint32(glm::uint32 _uint32)
{
    return std::_Bit_cast<Color>(_uint32);
}
