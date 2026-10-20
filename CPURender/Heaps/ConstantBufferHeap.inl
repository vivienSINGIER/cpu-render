#include "ConstantBufferHeap.h"

template <typename T>
void ConstantBufferHeap::Write(Handle _h, T const& _data)
{
    static_assert(_h.Size() == sizeof(T));
    
    memcpy(m_data.data(), &_data, sizeof(T));
}

template <typename T>
T const& ConstantBufferHeap::Read(Handle _h)
{
    static_assert(_h.Size() == sizeof(T));
    return *reinterpret_cast<const T*>(m_data.data() + _h.Offset());
}
