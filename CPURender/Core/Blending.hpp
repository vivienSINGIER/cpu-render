#ifndef CORE_BLENDING_HPP_DEFINED
#define CORE_BLENDING_HPP_DEFINED
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Blending
{
    enum Factor
    {
        Zero, One, SourceC, InvSourceC, SourceA, InvSourceA, DestC, InvDestC, DestA, InvDestA, SourceASaturate
    };
    
    enum Operation
    {
        Add, Subtract, InvSubtract, Min, Max
    };
    
    struct BlendType
    {
        Factor sourceFactor;
        Factor destFactor;
        Operation opp;
        
        Factor sourceAlphaFactor;
        Factor destAlphaFactor;
        Operation alphaOpp;
    };
    
    inline glm::vec4 GetFactor(glm::vec4 _src, glm::vec4 _dst, Factor _fact)
    {
        switch (_fact)
        {
        case Zero:              return glm::vec4(0.0);
        case One:               return glm::vec4(1.0f);
        case SourceC:           return _src;
        case InvSourceC:        return glm::vec4(1.0f) - _src;
        case SourceA:           return glm::vec4(_src.a);
        case InvSourceA:        return glm::vec4(1.0f - _src.a);
        case DestC:             return _dst;
        case InvDestC:          return glm::vec4(1.0f) -  _dst;
        case DestA:             return glm::vec4(_dst.a);
        case InvDestA:          return glm::vec4(1.0f - _dst.a);
        case SourceASaturate:   return glm::vec4(glm::min(_src.a, 1.0f - _dst.a));
        }
        return glm::vec4(1.0f);
    }
    
    inline glm::vec4 Op(glm::vec4 _src, glm::vec4 _dst, Operation _type)
    {
        switch (_type)
        {
            case Add: return _src + _dst;
            case Subtract: return _src - _dst;
            case InvSubtract: return _dst - _src;
            case Min: return glm::min(_src, _dst);
            case Max: return glm::max(_src, _dst);
        }
        return glm::vec4(1.0f);
    }
    
    inline glm::vec4 Blend(glm::vec4 _src, glm::vec4 _dst, BlendType _type)
    {
        glm::vec4 color = Op(_src * GetFactor(_src, _dst, _type.sourceFactor), _dst * GetFactor(_src, _dst, _type.destFactor), _type.opp);
        float alpha = Op(_src * GetFactor(_src, _dst, _type.sourceAlphaFactor), _dst * GetFactor(_src, _dst, _type.destAlphaFactor), _type.alphaOpp).a;

        // color is HDR, only keep it positive, clamping to 1 would clip highlights before tonemapping
        return glm::vec4(glm::max(glm::vec3(color), 0.0f), glm::clamp(alpha, 0.0f, 1.0f));
    }
    
    inline glm::vec4 BlendOpaque(glm::vec4 _src, glm::vec4 _dst)        { return Blend(_src, _dst, { One, Zero, Add,            One, Zero, Add } ); }
    inline glm::vec4 BlendAlpha(glm::vec4 _src, glm::vec4 _dst)         { return Blend(_src, _dst, { SourceA, InvSourceA, Add,  One, InvSourceA, Add } ); }
    inline glm::vec4 BlendPreMultAlpa(glm::vec4 _src, glm::vec4 _dst)   { return Blend(_src, _dst, { One, InvSourceA, Add,      One, InvSourceA, Add } ); }
    inline glm::vec4 BlendAdd(glm::vec4 _src, glm::vec4 _dst)           { return Blend(_src, _dst, { One, One, Add,             Zero, One, Add } ); }
    inline glm::vec4 BlendAddAlpha(glm::vec4 _src, glm::vec4 _dst)      { return Blend(_src, _dst, { SourceA, One, Add,         Zero, One, Add } ); }
    inline glm::vec4 BlendMult(glm::vec4 _src, glm::vec4 _dst)          { return Blend(_src, _dst, { DestC, Zero, Add,          Zero, One, Add } ); }
    inline glm::vec4 Blend2xMult(glm::vec4 _src, glm::vec4 _dst)        { return Blend(_src, _dst, { DestC, SourceC, Add,       Zero, One, Add } ); }
    inline glm::vec4 BlendScreen(glm::vec4 _src, glm::vec4 _dst)        { return Blend(_src, _dst, { One, InvSourceC, Add,      Zero, One, Add } ); }
    inline glm::vec4 BlendSubtractive(glm::vec4 _src, glm::vec4 _dst)   { return Blend(_src, _dst, { One, One, InvSubtract,     Zero, One, Add } ); }
    inline glm::vec4 BlendMin(glm::vec4 _src, glm::vec4 _dst)           { return Blend(_src, _dst, { One, One, Min,             Zero, One, Add } ); }
    inline glm::vec4 BlendMax(glm::vec4 _src, glm::vec4 _dst)           { return Blend(_src, _dst, { One, One, Max,             Zero, One, Add } ); }
    
    enum BlendMode
    {
        OPAQUE, ALPHA, PREMULTALPHA, ADDITIVE, ADDITIVE_ALPHA, MULTIPLICATIVE, MULTIPLICATIVE_2X, SCREEN, SUBTRACTIVE, MIN, MAX, COUNT
    };
    
    inline glm::vec4 Blend(glm::vec4 _src, glm::vec4 _dst, BlendMode _mode)
    {
        switch (_mode)
        {
        case OPAQUE:            return BlendOpaque(_src, _dst);
        case ALPHA:             return BlendAlpha(_src, _dst);
        case PREMULTALPHA:      return BlendPreMultAlpa(_src, _dst);
        case ADDITIVE:          return BlendAdd(_src, _dst);
        case ADDITIVE_ALPHA:    return BlendAddAlpha(_src, _dst);
        case MULTIPLICATIVE:    return BlendMult(_src, _dst);
        case MULTIPLICATIVE_2X: return Blend2xMult(_src, _dst);
        case SCREEN:            return BlendScreen(_src, _dst);
        case SUBTRACTIVE:       return BlendSubtractive(_src, _dst);
        case MIN:               return BlendMin(_src, _dst);
        case MAX:               return BlendMax(_src, _dst);
        }
        return BlendAlpha(_src, _dst);
    }
    
}

#endif
