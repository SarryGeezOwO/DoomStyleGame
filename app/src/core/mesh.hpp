#ifndef GZ_MESH_HPP
#define GZ_MESH_HPP

#include "util/common_types.hpp"
#include "renderer/vertex_buffer.hpp"
#include "renderer/index_buffer.hpp"
#include "renderer/vertex_array.hpp"
#include "renderer/vertex_layout.hpp"
#include "memory"
#include <string>

namespace Geez
{
    struct Mesh
    {
    friend struct MeshManager;
    protected:
        std::string m_id = "";

    private:
        std::unique_ptr<VertexBuffer> p_vbo;
        std::unique_ptr<IndexBuffer>  p_ebo;
        std::unique_ptr<VertexArray>  p_vao;

    public:
        U32 draw_mode;  //< GL_LINES, GL_TRIANGLES, GL_POINTS, etc.
        Mesh(const std::string& id, F32* vertices, U32 vertex_count, U32* indices, U32 index_count, const VertexBufferLayout& layout);
        Mesh(const std::string& id, F32* vertices, U32 vertex_count, U32* indices, U32 index_count, const VertexBufferLayout& layout, U32 draw);
        ~Mesh();

        // Invokes VAO.bind()
        void bind();

        inline const std::string& id() const { return m_id; }
        inline U32 index_count() const { return p_ebo->count(); }
        inline VertexBuffer* vbo() noexcept { return p_vbo.get(); }
        inline IndexBuffer*  ebo() noexcept { return p_ebo.get(); }
        inline VertexArray*  vao() noexcept { return p_vao.get(); }
    };
}

#endif