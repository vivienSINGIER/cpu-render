#ifndef BMP_PARSER_STRUCTS_HPP_DEFINED
#define BMP_PARSER_STRUCTS_HPP_DEFINED

#include <cstdint>
#include <glm/ext/scalar_uint_sized.hpp>

#include "../Datas/Color.h"

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;

#pragma pack(1)
struct BMPFileHeader
{
    unsigned char fileMarker[2];
    unsigned int size;
    uint16_t unused1;
    uint16_t unused2;
    unsigned int imageDataOffset;
};

#pragma pack(1)
struct DIBHeader
{
    uint32 dibHeaderSize;
    uint32 imgWidth;
    uint32 imgHeight;
    uint16 planes;
    uint16 bitsPerPixel;
    uint32 compression;
    uint32 imageSize;
    uint32 xPixPerMeter;
    uint32 yPixPerMeter;
    uint32 colorsInColorTable;
    uint32 importantColorCount;
    uint32 redBitMask;
    uint32 greenBitMask;
    uint32 blueBitMask;
    uint32 alphaBitMask;
    uint32 colorSpaceType;
    uint32 colorSpaceEndpoints;
    uint32 redGamma;
    uint32 greenGamma;
    uint32 blueGamma;
    uint32 intent;
    uint32 iccProfileData;
    uint32 iccProfileSize;
    uint32 unused;
};

struct ColorData
{
    virtual glm::vec4 ToVec4() = 0;
};

struct Color24 : public ColorData
{
    uint8 blue;
    uint8 green;
    uint8 red;
    
    int BitSize() { return 3; }

    glm::vec4 ToVec4(  )
    {
        return glm::vec4((float)red / 255.0f, (float)green / 255.0f, (float)blue / 255.0f, 1.0f);
    }
};

#endif