#ifndef BMPPARSER_INL_DEFINED
#define BMPPARSER_INL_DEFINED

#include <glm/exponential.hpp>

#include "BMPParser.hpp"
#include "../Datas/Texture.h"

template <typename PixelT>
void BMPParser::ParsePixelData()
{
    pixelData.resize(dibHeader.imgHeight * dibHeader.imgWidth * dibHeader.bitsPerPixel / 8);
    
    stream.seekg(header.imageDataOffset, std::ios::beg);
    
    stream.read(pixelData.data(), dibHeader.imgHeight * dibHeader.imgWidth * dibHeader.bitsPerPixel / 8);
}

template <typename PixelT>
void BMPParser::WritePixelData(Texture* _tex, glm::uvec2 _coord)
{
    PixelT* data = reinterpret_cast<PixelT*>(pixelData.data() + (dibHeader.bitsPerPixel / 8) * (dibHeader.imgWidth * _coord.y + _coord.x));
    
    PixelT instance;
    memcpy(&instance.blue, data, dibHeader.bitsPerPixel / 8);
    
    ColorData* c = reinterpret_cast<ColorData*>(&instance);
    
    glm::vec4 v = c->ToVec4();
    v.r = glm::pow(v.r, 2.2f);
    v.g = glm::pow(v.g, 2.2f);
    v.b = glm::pow(v.b, 2.2f);
    
    _tex->Store(_coord, v);
}

#endif
