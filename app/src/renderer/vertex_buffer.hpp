#ifndef GZ_VERTEX_BUFFER_HPP
#define GZ_VERTEX_BUFFER_HPP

#include "util/common_types.hpp"

namespace Geez
{
    struct VertexBuffer 
    {
    private:
        U32 m_id;

    public:
        VertexBuffer(const U32 size, const void* data);
        ~VertexBuffer();

        void bind();
        void unbind();

        inline U32 id() const { return m_id; }
    };
}

#endif