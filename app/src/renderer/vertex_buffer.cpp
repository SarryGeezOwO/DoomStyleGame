#include "vertex_buffer.hpp"
#include "util/error.hpp"
#include <gl/glew.h>

Geez::VertexBuffer::VertexBuffer(const U32 size, const void* data)
{
    GL(glGenBuffers(1, &m_id));
    GL(glBindBuffer(GL_ARRAY_BUFFER, m_id));
    GL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

Geez::VertexBuffer::~VertexBuffer()
{
    GL(glDeleteBuffers(1, &m_id));
}

void Geez::VertexBuffer::bind()
{
    GL(glBindBuffer(GL_ARRAY_BUFFER, m_id));
}

void Geez::VertexBuffer::unbind()
{
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
