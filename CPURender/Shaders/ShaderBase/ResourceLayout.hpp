#ifndef SHADER_SHADER_BASER_RESOURCE_LAYOUT_HPP_DEFINED
#define SHADER_SHADER_BASER_RESOURCE_LAYOUT_HPP_DEFINED

#include <array>

#include "../../Heaps/HeapUtils.hpp"

#define MAX_CB 8
#define MAX_TEX 16
#define MAX_UAV 8

// registers declared by every shader
#define CB_PER_PASS 0
#define CB_LIGHT_PASS 1
#define CB_PER_OBJECT 2
#define CB_CUSTOM 3


class Texture;

enum class UAVType { BUFFER, TEXTURE };

struct CBSlotDesc  { uint16_t size; bool isDeclared; bool isVS; bool isPS; };
struct TexSlotDesc { bool isDeclared; bool isOptional; bool isVS; bool isPS; };
struct UAVSlotDesc { UAVType type; uint16_t stride; bool isDeclared; };                                                                                                                                                           
                                                                                                                                                                                                                                    
struct ResourceLayout                                                                                                                                                                                                             
{                                                                                                                                                                                                                                 
    std::array<CBSlotDesc,  MAX_CB>  cb;                                                                                                                                                                                          
    std::array<TexSlotDesc, MAX_TEX> tex;                                                                                                                                                                                         
    std::array<UAVSlotDesc, MAX_UAV> uav;      
}; 

struct CBRegister  { uint16_t size; bool isDeclared; };
struct TexRegister { bool isDeclared; bool isOptional; };
struct UAVRegister { UAVType type; uint16_t stride; bool isDeclared; };

struct ShaderResources
{
    std::array<uint8_t const*, MAX_CB> cbs{};
    std::array<Texture const*, MAX_TEX> textures{};
    std::array<UAVView, MAX_UAV> uavs{};
    
};

#endif
