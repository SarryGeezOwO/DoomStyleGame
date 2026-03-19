#ifndef GZ_VERTEX_ARRAY_HPP
#define GZ_VERTEX_ARRAY_HPP

#include "util/common_types.hpp"
#include "index_buffer.hpp"
#include "vertex_buffer.hpp"
#include "vertex_layout.hpp"
#include <vector>

namespace Geez
{
    struct VertexArray
    {
    private:
        U32 m_id;

    public:
        VertexArray();
        ~VertexArray();
        
        void bind();
        void unbind();
        void bind_buffer(VertexBuffer* vbo, IndexBuffer* ebo, const VertexBufferLayout& layout);

        inline U32 id() const { return m_id; }
    };
}

#endif