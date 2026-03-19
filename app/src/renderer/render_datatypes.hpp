#ifndef GZ_RENDER_DATATYPES_HPP
#define GZ_RENDER_DATATYPES_HPP

#include "gmp/gmp_types.hpp"
#include "resource/resource.hpp"
#include "util/common_types.hpp"
#include <glm/glm.hpp>
#include <memory>

/*
    For reasons, true structs [Structs that literally contain data]
        will be labeled with '_t' on the end. Don't ask why...

    Leave Texture_ID empty if no texture to use or whatever...
*/

namespace Geez
{
    enum RenderType {
        R_NONE = -1,    // Skipped at rendering
        R_WALL, 
        R_SECTOR
    };

    // pure Virtual
    // Global things needed
    struct IRenderData {
        friend struct Renderer;
    protected:
        RenderType type = R_NONE;

    public:
        ResourceID shader_id    = "";
    };

// ================================================= //
//                   SPECIFIC DATA                   //
// ================================================= //

    struct RenderWallData_t   : IRenderData {
        RenderWallData_t() { type = R_WALL; }

        glm::vec2 a; 
        glm::vec2 b;
        glm::vec2 ref_center; 
        F32 yBottom; 
        F32 yTop;
        bool flipped;
        bool debug_line;
        ResourceID texture_id   = "";
    };

    struct RenderSectorData_t : IRenderData {
        RenderSectorData_t() { type = R_SECTOR; }
        
        U32 index_count;
        F32 floor;
        F32 ceil;
        std::weak_ptr<sector_mesh_t> mesh;
        ResourceID texture_id_flor = "";
        ResourceID texture_id_ceil = "";
    };
}

#endif