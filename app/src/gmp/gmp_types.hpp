#ifndef GZ_GMP_TYPES_HPP
#define GZ_GMP_TYPES_HPP

#include "core/tag.hpp"
#include "util/common_types.hpp"
#include "resource/resource.hpp"
#include "renderer/vertex_array.hpp"
#include "renderer/vertex_buffer.hpp"
#include "renderer/index_buffer.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <array>

namespace Geez
{
    using Point_t = std::array<F32, 2>;
    using Polygon_t = std::vector<Point_t>;
    // Haha, batman, you cannnot stop me from writing bad code 🗿🗿

    struct decal_t : ITagClient {
        U32 id;
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 size;
        ResourceID texture_id;

        enum target_t {
            WALL, FLOOR, CEIL
        };

        target_t target;
        U32 target_id;
    };

    struct wall_t : ITagClient {
        U32 id;
        Point_t point_a;
        Point_t point_b;
        bool is_portal;
        ResourceID texture_id = "Wall";
        std::array<I32, 2> connected_sectors = {-1, -1}; // Ids of SectorA, SectorB
        U16 tagList = 0;

        // The Floor region normal if is_portal is true,
        // this also returns the solid wall normal if is_portal is false
        inline glm::vec3 normal_front() const {
            return m_normal[0];
        }

        // The Ceil region normal if is_portal is true
        // asserts false if is_portal is false
        inline glm::vec3 normal_back() const {
            assert(is_portal);
            return m_normal[1];
        }
        
    friend struct GeezMapData;
    protected: // Used for buffer generation
        std::vector<U32> scary_sectors = {};
        std::array<glm::vec3, 2> m_normal = {};
    };

    struct sector_t : ITagClient {
        U32 id;
        F32 floor_height;                           // Floor...
        F32 ceil_height;                            // Not relative to the floor_height
        bool is_interior;                           // Is this sector inside another sector
        glm::vec2 center = glm::vec2(0);            // Center for the polygon (not accounting for holes)
        std::vector<U32> walls = {};                // Wall_ids
        std::vector<Polygon_t> polygons = {{}};     // For Earcut
        ResourceID texture_id_floor = "Floor";      // Texture for the Floor mesh
        ResourceID texture_id_ceil  = "Ceil";       // Texture for the ceil mesh
    };

    struct sector_mesh_t {
        std::unique_ptr<VertexBuffer> vbo;
        std::unique_ptr<IndexBuffer> ebo;
        std::unique_ptr<VertexArray> vao;
    };
}

#endif