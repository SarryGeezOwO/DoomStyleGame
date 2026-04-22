#include "vertex_array.hpp"
#include "util/error.hpp"
#include <gl/glew.h>

Geez::VertexArray::VertexArray()
{
    GL(glGenVertexArrays(1, &m_id));
}

Geez::VertexArray::~VertexArray()
{
    GL(glDeleteVertexArrays(1, &m_id));
}

void Geez::VertexArray::bind_buffer(VertexBuffer *vbo, IndexBuffer *ebo, const VertexBufferLayout& layout)
{
    bind();
    vbo->bind();
    if (ebo) ebo->bind();
    else {
        GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); // Force remove EBO from the current state
    }

    U32 index = 0;
    U32 offset = 0;
    for(const VertexElement& ele : layout.elements()) {
        GL(glVertexAttribPointer(
            index, 
            ele.count, 
            ele.type, 
            ele.normalized, 
            layout.stride(), 
            reinterpret_cast<const void*>(offset)
        )); 
        GL(glEnableVertexAttribArray(index++));
        offset += ele.count * VertexElement::get_size(ele.type);
    }
}

void Geez::VertexArray::bind()
{
    GL(glBindVertexArray(m_id));
}

void Geez::VertexArray::unbind()
{
    GL(glBindVertexArray(0));
}