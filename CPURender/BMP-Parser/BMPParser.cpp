#include "BMPParser.hpp"

#include "../Core/GraphicsContext.h"

#include "../Factories/TextureFactory.h"
#include <iostream>

void BMPParser::Init(std::string const& _path)
{
    stream.open(_path.c_str(), std::ios::binary | std::ios::in);
}

Handle BMPParser::Parse()
{
    stream.read(reinterpret_cast<char*>(&header), sizeof(header));
    stream.read(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));
    
    stream.seekg(sizeof(header) + dibHeader.dibHeaderSize);
    
    Handle texH = TextureFactory::Instance()->CreateTexture("ParsedTexture", dibHeader.imgWidth, dibHeader.imgHeight);
    Texture* tex = TextureFactory::Instance()->GetTexture(texH);
    
    switch (dibHeader.bitsPerPixel)
    {
    case 24:
        ParsePixelData<Color24>(); break;
    default: assert(false && "Unsupported color bit size");
    }
    
    for (uint32 i = 0; i < dibHeader.imgHeight; i++)
    {
        for (uint32 j = 0; j < dibHeader.imgWidth; j++)
        {
            WritePixelData<Color24>(tex, {j, i});
        }
    }
    
    return texH;
}

void BMPParser::PrintSize()
{
    std::cout << "SIZE : " << header.size << "\n";
}

void BMPParser::PrintDimensions()
{
    std::cout << "DIMENSIONS : (" << dibHeader.imgWidth << ", " << dibHeader.imgHeight << ")\n";
}
