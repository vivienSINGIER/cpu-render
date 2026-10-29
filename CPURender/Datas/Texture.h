#ifndef TEXTURE_H_DEFINED
#define TEXTURE_H_DEFINED

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/ext/scalar_uint_sized.hpp>

#include "Color.h"

using namespace glm;

class Texture
{
public:
    enum Encoding { MORTON, LINEAR };
    enum Format { RGBA32, R32F };
    
    Texture(uint32 _width = 256, uint32 _height = 256, Format _format = RGBA32, Encoding _enc = MORTON, vec4 _color = Color::BLACK);
    ~Texture() = default;

    Texture ToLinear() const;
    
    void Load(std::string const& _filePath, bool _isSrgb = false);
    
    Format GetFormat()                              const;
    uint32 Size()                                   const;
    uint32 Width()                                  const;
    uint32 Height()                                 const;
    uint32 GetSize(uint32& _width, uint32& _height) const;
    void SetSize(uint32 _width, uint32 _height);
    
    glm::vec4 SampleNP(glm::vec2 uv) const; // nearest point, format aware
    glm::vec4 SampleNPCube(glm::vec3 _dir) const; 
    glm::vec4 SampleNPCubeLod(glm::vec3 _dir, float _lod) const;
    
    // vec4 is always (r, g, b, a)
    glm::vec4   Load(uvec2 _c) const;
    glm::vec4   Load(uint32 _x, uint32 _y) const;
    void        Store(uvec2 _c, glm::vec4 _v);
    void        Store(uint32 _x, uint32 _y, glm::vec4 _v);
    
    void StoreCube(uint32 _lod, uint32 _face, uint32 _x, uint32 _y, vec4 _c);
    void StoreCube(uint32 _lod, uint32 _face, uvec2 _uv, vec4 _c);
    
    void        Clear(glm::vec4 _color);
    
    template <typename T>
    T GetPixelData(uint32 _x, uint32 _y);
    template <typename T>
    void SetPixelData(uint32 _x, uint32 _y, T _data);
    
    vec4* GetData();
    
    void GenerateCubeMips();
    vec3 CubeFaceUVToDir(uint32 _face, vec2 _uv);
    bool TexelToCubeDir(uint32_t _x, uint32_t _y, vec3& _outDir);
    
    static void BakeIrradiance(Texture* _src, Texture* _dst, uint32_t _faceSize);
    
private:
    std::vector<vec4> m_data;
    uint32 m_width;
    uint32 m_height;
    uint32 m_size;
    
    Encoding m_encoding;
    Format m_format;
    
    int m_mipMapLast = 6;
    
    uint32 GetIndex(uint32 _x, uint32 _y) const;
    
    uint32 CubeFaceSize() const;
    uint32 LodCount() const;
    uint32 MaxCubeLod() const;
    uvec2 CubeTexel(uint32 _lod, uint32 _face, uint32 _x, uint32 _y) const;
    vec2 GetCubeSampleUV(vec3 _dir, uint32& _faceIndex) const;
};

#include "Texture.inl"

#endif
