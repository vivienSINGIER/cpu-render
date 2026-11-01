#ifndef SHADER_BASE_MATERIAL_INL_DEFINED
#define SHADER_BASE_MATERIAL_INL_DEFINED

#include "Material.h"

#include "../../Core/GraphicsContext.h"
#include "ShaderGroup.h"

template <typename T>
void MaterialData::SetParams(uint32_t _slot, T const& _value)
{
    assert(_slot >= CB_CUSTOM && _slot < MAX_CB && "Constant buffer slot index out of bounds or reserved for the renderer");

    CBSlotDesc const& l = m_layout.cb[_slot];

    assert(l.isDeclared && l.size == sizeof(T) && "cannot fill constant buffer with non matching format");

    m_ctx->constantBufferHeap.Write(m_cbs[_slot], _value);
    m_cbWritten |= 1u << _slot;
}

#endif
