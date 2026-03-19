#ifndef VERTEX_LAYOUT_HPP
#define VERTEX_LAYOUT_HPP

#include "util/log.hpp"
#include "util/common_types.hpp"
#include <vector>
#include <gl/glew.h>

namespace Geez
{

    struct VertexElement
    {
        U32 count;
        U32 type;
        U32 normalized;

        static U32 get_size(U32 type) {
            switch (type)
            {
            case GL_FLOAT:           return sizeof(GLfloat);
            case GL_UNSIGNED_INT:    return sizeof(GLuint);
            default:
                GZ_LOG(GZ_FATAL, "Cannot get size of Unknown type.");
                break;
            }
            return 0;
        }
    };

    // -----------------------------------------------------------------
    // -----------------------------------------------------------------

    struct VertexBufferLayout
    {
    private:
        std::vector<VertexElement> m_elements;
        U32 m_stride = 0;
     
    public:
        template<typename T>
        void push_attr(U32 count, U32 normalize) {
            static_assert(false, "Invalid type for VAO.");
        }

        inline U32 stride() const { return m_stride; }
        inline const std::vector<VertexElement>& elements() const { return m_elements; }
    };

    // --------------------Type Implementations-------------------------
    // -----------------------------------------------------------------

    template <>
    inline void VertexBufferLayout::push_attr<F32>(U32 count, U32 normalize) {
        m_elements.push_back({count, GL_FLOAT, normalize});
        m_stride += count * VertexElement::get_size(GL_FLOAT);
    }

    template <>
    inline void VertexBufferLayout::push_attr<U32>(U32 count, U32 normalize) {
        m_elements.push_back({count, GL_UNSIGNED_INT, normalize});
        m_stride += count * VertexElement::get_size(GL_UNSIGNED_INT);
    }
}

#endif