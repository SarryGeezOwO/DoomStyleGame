#ifndef GZ_INDEX_BUFFER_HPP
#define GZ_INDEX_BUFFER_HPP

#include "util/common_types.hpp"

namespace Geez
{
    struct IndexBuffer
    {
    private:
        U32 m_id;
        U32 m_count;

    public:
        IndexBuffer(const U32 count, const void* data);
        ~IndexBuffer();

        void bind();
        void unbind();
    
        inline U32 id() const { return m_id; }
        inline U32 count() const { return m_count; }
    };
}

#endif