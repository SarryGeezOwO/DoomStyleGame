#include "mesh_manager.hpp"
#include "util/log.hpp"
#include "util/error.hpp"
#include "renderer/vertex_layout.hpp"
#include <gl/glew.h>

/*
    [2026-03-12]
    I have no shitting clue on what this class 
    is for, as far as I am concerned, doom has only like rectangles?
    so errmmm, no clue... I Guess cubes are good for some cases instead of 
    like drawing 6 faces manually. Sectors are also not utilizing this, maybe I
    will need to submit sector meshes here for painter's algorithm but who knows?
*/

void Geez::MeshManager::create_quad()
{
    // Vertex: X, Y, Z   S, T,  Normal
    F32 vertices[32] = {
        -0.5f, -0.5f, 0,    0.0f, 0.0f,     0,0,1,
         0.5f, -0.5f, 0,    1.0f, 0.0f,     0,0,1,
         0.5f,  0.5f, 0,    1.0f, 1.0f,     0,0,1,
        -0.5f,  0.5f, 0,    0.0f, 1.0f,     0,0,1,
    };
    U32 indices[6] = {
        0, 1, 2,   // first triangle
        2, 3, 0    // second triangle
    };

    VertexBufferLayout layout;
    layout.push_attr<F32>(3, GL_FALSE);
    layout.push_attr<F32>(2, GL_FALSE);
    layout.push_attr<F32>(3, GL_FALSE);
    create("QUAD", vertices, 32, indices, 6, layout, GL_TRIANGLES);
    GZ_LOG(GZ_SUCCESS, "Primitive Mesh [QUAD] is created.");
}

void Geez::MeshManager::create_line()
{
    // Vertex: X, Y, Z 
    F32 vertices[6] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    VertexBufferLayout layout;
    layout.push_attr<F32>(3, GL_FALSE);
    create("LINE", vertices, 6, nullptr, 0, layout, GL_LINES);
    GZ_LOG(GZ_SUCCESS, "Primitive Mesh [LINE] is created.");
}

void Geez::MeshManager::create_axis()
{
    // Vertex: X, Y, Z
    F32 vertices[12] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    U32 indices[6] = {
        0, 1, // X
        0, 2, // Y
        0, 3  // Z
    };

    VertexBufferLayout layout;
    layout.push_attr<F32>(3, GL_FALSE);
    create("AXIS", vertices, 12, indices, 6, layout, GL_LINES);
    GZ_LOG(GZ_SUCCESS, "Primitive Mesh [AXIS] is created.");
}

void Geez::MeshManager::create_plane()
{
    // Vertex: X, Y, Z   S, T     Normal
    F32 vertices[32] = {
        -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,    0.0f, 1.0f, 0.0f,
         0.5f, 0.0f, -0.5f,   1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,   1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
        -0.5f, 0.0f,  0.5f,   0.0f, 1.0f,    0.0f, 1.0f, 0.0f,
    };

    U32 indices[6] = {
        0, 1, 2,
        2, 3, 0
    };

    VertexBufferLayout layout;
    layout.push_attr<F32>(3, GL_FALSE);
    layout.push_attr<F32>(2, GL_FALSE);
    layout.push_attr<F32>(3, GL_FALSE);
    create("PLANE", vertices, 32, indices, 6, layout, GL_TRIANGLES);
    GZ_LOG(GZ_SUCCESS, "Primitive Mesh [PLANE] is created.");
}

Geez::MeshManager::~MeshManager()
{
    GZ_LOG(GZ_DEBUG, "Destroying [%d] meshes.", m_meshes.size());
    Internal::Logger::increment_tab_level(1);
    m_meshes.clear();
    Internal::Logger::decrement_tab_level(1);
    GZ_LOG(GZ_DEBUG, "Mesh Manager quit.");
}

Geez::MeshManager::MeshManager(std::vector<Geez::Internal::PrimitveMesh> &&primitives)
{
    GZ_LOG(GZ_DEBUG, "Creating Primitive meshes...");
    Internal::Logger::increment_tab_level(1);
    for (auto p : primitives) {
        switch (p) {
            case Internal::QUAD:  create_quad(); break;
            case Internal::LINE:  create_line(); break;
            case Internal::AXIS:  create_axis(); break;
            case Internal::PLANE: create_plane(); break;
        }
    }
    unbind();
    Internal::Logger::decrement_tab_level(1);
}

void Geez::MeshManager::bind(const std::string &mesh_id)
{
    Mesh* m = get(mesh_id);
    if (m != nullptr) {
        m->bind();
    }
}

void Geez::MeshManager::unbind()
{
    GL(glBindVertexArray(0));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

bool Geez::MeshManager::exists(const std::string &mesh_id) const
{
    return m_meshes.find(mesh_id) != m_meshes.end();
}

void Geez::MeshManager::create(const std::string& mesh_id, F32 *vertices, U32 vertex_count, U32 *indices, U32 index_count, const VertexBufferLayout &layout, U32 draw_mode)
{
    if (exists(mesh_id)) {
        GZ_LOG(GZ_WARNING, "[Creating] Mesh [%s] already exists, overriding old mesh.", mesh_id.c_str());
    }

    std::unique_ptr<Mesh> m = std::make_unique<Mesh>(mesh_id, vertices, vertex_count, indices, index_count, layout, draw_mode);
    m_meshes[mesh_id] = std::move(m);
}

void Geez::MeshManager::create(const std::string& mesh_id, F32 *vertices, U32 vertex_count, U32 *indices, U32 index_count, const VertexBufferLayout &layout)
{
    create(mesh_id, vertices, vertex_count, indices, index_count, layout, GL_TRIANGLES);
}

void Geez::MeshManager::create(const std::string& mesh_id, std::vector<F32> vertices, std::vector<U32> indices, const VertexBufferLayout &layout, U32 draw_mode)
{
    create(mesh_id, vertices.data(), vertices.size(), indices.data(), indices.size(), layout, draw_mode);
}

void Geez::MeshManager::create(const std::string& mesh_id, std::vector<F32> vertices, std::vector<U32> indices, const VertexBufferLayout &layout)
{
    create(mesh_id, vertices.data(), vertices.size(), indices.data(), indices.size(), layout, GL_TRIANGLES);
}

void Geez::MeshManager::remove(const std::string& mesh_id)
{
    m_meshes.erase(mesh_id);
}

Geez::Mesh* Geez::MeshManager::get(const std::string& mesh_id) noexcept
{
    if (exists(mesh_id)) {
        return m_meshes.at(mesh_id).get();
    }
    return nullptr;
}
