#ifndef BMP_PARSER_BMPPARSER_H_DEFINED
#define BMP_PARSER_BMPPARSER_H_DEFINED
#include <fstream>
#include <iosfwd>
#include <vector>
#include <glm/vec2.hpp>

#include "Structs.hpp"

#include "../Heaps/Heap.h"

class Texture;
class GraphicsContext;

class BMPParser
{
public:
    void Init(std::string const& _path);
    
    Handle Parse();
    
    void PrintSize();
    void PrintDimensions();
    
    template<typename PixelT>
    void ParsePixelData();
    
    template<typename PixelT>
    void WritePixelData(Texture* _tex, glm::uvec2 _coord);
    
private:
    BMPFileHeader header;
    DIBHeader dibHeader;
    
    std::ifstream stream;
    
    std::vector<char> pixelData;
};

#include "BMParser.inl"

#endif
