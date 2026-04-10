#ifndef GZ_RENDERER_HPP
#define GZ_RENDERER_HPP

#include "gmp/gmp_types.hpp"
#include "gmp/gmp.hpp"
#include "core/window.hpp"
#include "core/game_object_manager.hpp"
#include "core/mesh_manager.hpp"
#include "camera.hpp"
#include "render_datatypes.hpp"
#include "render_strategy.hpp" 
#include "resource/resource_manager.hpp"
#include "util/common_types.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <memory>

namespace Geez
{
    struct RenderContext {
        Camera* active_camera = nullptr;
        Window* active_window = nullptr;
        MeshManager*        meshes      = nullptr; 
        ResourceManager*    resources   = nullptr;

        // Force Logs on missing stuffs
        bool validate() const;
    };

    struct Renderer
    {
    private:
        std::array<std::vector<std::unique_ptr<IRenderData>>, RenderType::R_RENDER_TYPE_COUNT> render_buckets;
        std::unique_ptr<IRenderStrategy> strategies[RenderType::R_RENDER_TYPE_COUNT];
        std::unique_ptr<RenderContext> context;

    public:
        Renderer();
        ~Renderer();

        inline void set_context(std::unique_ptr<RenderContext> cntx) {
            context = std::move(cntx);
        }

        inline const RenderContext& get_context() const {
            return *context.get();
        } 

        // Submit all walls and sectors from a given map data
        void submit_map_geometry(GeezMapData& map);

        // Submit any RenderDataTypes here
        void submit(std::unique_ptr<IRenderData> data);
        
        // Renders until queue is empty
        void flush();
    };


    /* ===== DEBUG DRAWS ===== */
    namespace Internal 
    {
    #ifdef GZ_BUILD_DEBUG
        /// Usage: GZ_DEBUG_DRAW_RAY(r_ctx, pos, dir, length, thickness, color)
        /// @param r_cntx The RenderContext to use
        /// @param pos World position where the ray starts
        /// @param dir is not required to be normalized
        /// @param length how long this ray shoots, defaults to 0.1f
        /// @param thickness line draw thickness, defaults to 2.0f
        /// @param color line draw color, defaults to white
        #define GZ_DEBUG_DRAW_RAY(...) Geez::Internal::debug_draw_ray(__VA_ARGS__)
    #else
        // No-op in non-debug builds (accepts any args)
        #define GZ_DEBUG_DRAW_RAY(...) ((void)0)
    #endif

        void debug_draw_ray(
            const RenderContext& context,
            const glm::vec3& world_position,
            const glm::vec3& direction,
            F32 length = 0.1f, 
            F32 thickness = 2,
            const Color3f color = Color3f(1)
        );
    }
}

#endif