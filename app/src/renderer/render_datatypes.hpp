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

// Not including R_NONE
#define GZ_RENDER_TYPES_COUNT 4

namespace Geez
{
    enum RenderType {
        R_NONE = -1,    // Skipped at rendering
        R_WALL, 
        R_SECTOR,
        R_GAMEOBJECT,
        R_GUI           // Not including Texts
    };

    // pure Virtual
    // Global things needed
    struct IRenderData {
        friend struct Renderer;
    protected:
        RenderType type = R_NONE;

    public:    
        ResourceID shader_id    = "";
        ResourceID texture_ids[8]; // 8 texture slots avail or something
    };

// ================================================= //
//                   SPECIFIC DATA                   //
// ================================================= //

    struct RenderDataWall_t   : IRenderData {
        RenderDataWall_t() { type = R_WALL; }

        glm::vec2 a; 
        glm::vec2 b;
        glm::vec2 ref_center; 
        F32 yBottom; 
        F32 yTop;
        bool flipped;
        bool debug_line;
    };

    struct RenderDataSector_t : IRenderData {
        RenderDataSector_t() { type = R_SECTOR; }
        
        U32 index_count;
        F32 floor;
        F32 ceil;
        std::weak_ptr<sector_mesh_t> mesh;
        // texture   0-floor   1-ceil
    };

    struct RenderDataGameobject_t : IRenderData {
        RenderDataGameobject_t() { type = R_GAMEOBJECT; }

        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };

    struct RenderDataGUI_t : IRenderData {
        RenderDataGUI_t() { type = R_GUI; }
        
        glm::vec2 screen_pos;
        glm::vec2 size;
        F32 angle;
    };
}

#endif