#include "index_buffer.hpp"
#include "util/error.hpp"
#include <gl/glew.h>

Geez::IndexBuffer::IndexBuffer(const U32 count, const void* data) 
    : m_count(count)
{
    GL(glGenBuffers(1, &m_id));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
    GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data, GL_STATIC_DRAW));
}

Geez::IndexBuffer::~IndexBuffer() 
{
    GL(glDeleteBuffers(1, &m_id));
}

void Geez::IndexBuffer::bind() 
{
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
}

void Geez::IndexBuffer::unbind() 
{
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}