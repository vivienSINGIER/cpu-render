#ifndef COLOR_H_DEFINED
#define COLOR_H_DEFINED

#include <glm/ext/scalar_uint_sized.hpp>
#include <bit>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Color
{
    inline static glm::vec4 WHITE      = {1.0f, 1.0f, 1.0f, 1.0f};
    inline static glm::vec4 BLACK      = {0.0f, 0.0f, 0.0f, 1.0f};
    inline static glm::vec4 RED        = {1.0f, 0.0f, 0.0f, 1.0f};
    inline static glm::vec4 GREEN      = {0.0f, 1.0f, 0.0f, 1.0f};
    inline static glm::vec4 BLUE       = {0.0f, 0.0f, 1.0f, 1.0f};
    inline static glm::vec4 YELLOW     = {1.0f, 1.0f, 0.0f, 1.0f};
    inline static glm::vec4 CYAN       = {0.0f, 1.0f, 1.0f, 1.0f};
    inline static glm::vec4 MAGENTA    = {1.0f, 0.0f, 1.0f, 1.0f};
    inline static glm::vec4 ORANGE     = {1.0f, 0.65f, 0.0f, 1.0f};
    inline static glm::vec4 PURPLE     = {0.5f, 0.0f, 0.5f, 1.0f};
    inline static glm::vec4 PINK       = {1.0f, 0.75f, 0.80f, 1.0f};
    inline static glm::vec4 BROWN      = {0.545f, 0.271f, 0.075f, 1.0f};
    inline static glm::vec4 LIME       = {0.196f, 0.804f, 0.196f, 1.0f};
    inline static glm::vec4 TEAL       = {0.0f, 0.5f, 0.5f, 1.0f};
    inline static glm::vec4 NAVY       = {0.0f, 0.0f, 0.5f, 1.0f};
    inline static glm::vec4 MAROON     = {0.5f, 0.0f, 0.0f, 1.0f};
    inline static glm::vec4 OLIVE      = {0.5f, 0.5f, 0.0f, 1.0f};
    inline static glm::vec4 GRAY       = {0.5f, 0.5f, 0.5f, 1.0f};
    inline static glm::vec4 LIGHT_GRAY = {0.75f, 0.75f, 0.75f, 1.0f};
    inline static glm::vec4 DARK_GRAY  = {0.25f, 0.25f, 0.25f, 1.0f};
    inline static glm::vec4 NONE       = {0.0f, 0.0f, 0.0f, 0.0f};
};

#endif