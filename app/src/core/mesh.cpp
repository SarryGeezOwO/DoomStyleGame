#include "mesh.hpp"
#include "util/log.hpp"
#include "util/error.hpp"
#include "gl/glew.h"

Geez::Mesh::Mesh(const std::string& id, F32 *vertices, U32 vertex_count, U32 *indices, U32 index_count, const VertexBufferLayout& layout)
    :m_id(id)
{
    if (vertices == nullptr) {
        GZ_LOG_FORCE(GZ_FAIL, "Cannot create mesh from missing Vertices [nullptr]");
        return;
    }

    // A Mesh can have no EBO
    // Passing a nullptr on 'indices' will skip EBO creation
    p_vbo = std::make_unique<VertexBuffer>(vertex_count * sizeof(F32), vertices);
    if (indices != nullptr)
        p_ebo = std::make_unique<IndexBuffer>(index_count, indices);

    p_vao = std::make_unique<VertexArray>();
    p_vao->bind_buffer(p_vbo.get(), p_ebo.get(), layout);
    p_vao->unbind();
    p_vbo->unbind();
    if (p_ebo) p_ebo->unbind();
    draw_mode = GL_TRIANGLES; // Default
}

Geez::Mesh::Mesh(const std::string& id, F32 *vertices, U32 vertex_count, U32 *indices, U32 index_count, const VertexBufferLayout &layout, U32 draw)
    : Mesh(id, vertices, vertex_count, indices, index_count, layout)
{
    draw_mode = draw;
}

Geez::Mesh::~Mesh()
{
    p_vbo.reset();
    p_ebo.reset();
    p_vao.reset();
    GZ_LOG(GZ_SUCCESS, "Mesh [%s] destroyed.", m_id.c_str());
}

void Geez::Mesh::bind()
{
    p_vao->bind();
}
