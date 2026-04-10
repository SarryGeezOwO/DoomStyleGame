#ifndef GZ_MESH_MANAGER
#define GZ_MESH_MANAGER

#include "util/common_types.hpp"
#include "mesh.hpp"
#include <vector>
#include <memory>
#include <unordered_map>

/*
    PrimitveMeshes: "This shit is required!"
    also PrimitiveMeshes: "yeah, you can choose not to create it..."

    Don't ask why... 
    (There's actually a resaon why)
*/

namespace Geez
{
    namespace Internal
    {
        enum PrimitveMesh 
        {
            // Required*
            QUAD,   //< A flat plane that expands on X and Y, origin at center  (required to render walls)
            LINE,   //< A line that stretches at Z, origin at point_A           (required by draw_ray, draw_line)
            DECAL,  //< A modified quad (required for rendering decals, duh)


            // Optionals
            PLANE,  //< A Flat plane that expands on X and Z, origin at center
        };
    }

    struct MeshManager
    {
    private:
        std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;

        void create_quad();
        void create_line();
        void create_decal();
        void create_plane();

    public:
        
        /// @brief Constructs the MeshManager and initializes primitive meshes
        /// @param primitives Vector of primitive mesh types to create
        MeshManager(std::vector<Internal::PrimitveMesh>&& primitives);
        ~MeshManager();

        /// @brief Binds a mesh for rendering
        /// @param mesh_id The unique identifier of the mesh to bind
        void bind(const std::string& mesh_id);
        
        /// @brief Unbinds the currently bound mesh
        void unbind();
        
        /// @brief Checks if a mesh with the given ID exists
        /// @param mesh_id The unique identifier to check
        /// @return True if the mesh exists, false otherwise
        bool exists(const std::string& mesh_id) const;

        void create(const std::string& mesh_id, F32* vertices, U32 vertex_count, U32* indices, U32 index_count, const VertexBufferLayout& layout, U32 draw_mode);
        void create(const std::string& mesh_id, F32* vertices, U32 vertex_count, U32* indices, U32 index_count, const VertexBufferLayout& layout);

        void create(const std::string& mesh_id, std::vector<F32> vertices, std::vector<U32> indices, const VertexBufferLayout& layout, U32 draw_mode);
        void create(const std::string& mesh_id, std::vector<F32> vertices, std::vector<U32> indices, const VertexBufferLayout& layout);
        
        /// @brief Removes a mesh from the manager
        /// @param mesh_id The unique identifier of the mesh to remove
        void remove(const std::string& mesh_id);

        /// @brief Retrieves a mesh by its ID
        /// @param mesh_id The unique identifier of the mesh
        /// @return Pointer to the mesh, or nullptr if not found
        Mesh* get(const std::string& mesh_id) noexcept;
    };
}

#endif