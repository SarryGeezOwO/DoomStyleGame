#include <renderer.hpp>
#include <SDL3/SDL.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl3.h"
#include "util/error.hpp"
#include "util/log.hpp"
#include "util/geometry_util.hpp"
#include <gl/glew.h>
#include <algorithm>

using namespace glm;

// TEMP
static const char* decal_shader = "decal";
static const char* unlit_shader = "unlit";
static const char* lit_shader   = "lit_texture";

bool Geez::RenderContext::validate() const
{
    if (!active_camera) {
        GZ_LOG_FORCE(GZ_FAIL, "[RENDERER] RenderContext missing active_camera.");
    }

    if (!active_window) {
        GZ_LOG_FORCE(GZ_FAIL, "[RENDERER] RenderContext missing active_window.");
    }

    if (!meshes) {
        GZ_LOG_FORCE(GZ_FAIL, "[RENDERER] RenderContext missing meshes.");
    }

    if (!resources) {
        GZ_LOG_FORCE(GZ_FAIL, "[RENDERER] RenderContext missing resources.");
    }

    return         
        active_camera   &&
        active_window   &&
        meshes          &&
        resources;
}


Geez::Renderer::Renderer()
{
    // Load Strategies here
    strategies[R_WALL]       = std::make_unique<RenderStrategyWall>();
    strategies[R_SECTOR]     = std::make_unique<RenderStrategySector>();
    strategies[R_DECAL]      = std::make_unique<RenderStrategyDecal>();
    strategies[R_GAMEOBJECT] = std::make_unique<RenderStrategyGameobject>();
    strategies[R_GUI]        = std::make_unique<RenderStrategyGUI>();
}

Geez::Renderer::~Renderer()
{
    GZ_LOG(GZ_DEBUG, "Renderer quit");
}

void Geez::Renderer::submit_map_geometry(GeezMapData &map)
{
    // Decals
    for (const decal_t& decal : map.get_decals()) {
        auto r_decal_data = [&]{
            RenderDataDecal_t d{};
            d.position       = decal.position;
            d.normal         = decal.normal;
            d.size           = decal.size;
            d.texture_ids[0] = decal.texture_id;
            d.shader_id      = decal_shader;
            d.target         = decal.target;
            return d;
        }();
        submit(std::make_unique<RenderDataDecal_t>(r_decal_data));
    }

    // Wall_ID, Sector_ID
    std::unordered_map<U32, U32> portal_prev_sec;
    const vec2 camPosXZ(context->active_camera->position.x, context->active_camera->position.y);

    // Map geometry or something
    for (const sector_t& sector : map.get_sectors()) {

        for (const U32 wall_id : sector.walls) {
            const wall_t& wall = *map.get_wall(wall_id);
            const vec2 point_a = {wall.point_a[0], wall.point_a[1]};
            const vec2 point_b = {wall.point_b[0], wall.point_b[1]};

            auto r_wall_data = [&]{
                RenderDataWall_t d{};
                d.flipped        = false;
                d.shader_id      = lit_shader;
                d.texture_ids[0] = wall.texture_id;
                d.ref_center     = sector.center;
                d.a              = point_a;
                d.b              = point_b;
            #ifdef GZ_BUILD_DEBUG
                d.debug_is_floor = true;
                d.debug_is_ceil  = true;
            #endif
                return d;
            }();

            // Render full quad from ceil to floor
            if (!wall.is_portal) {
                r_wall_data.yBottom = sector.floor_height;
                r_wall_data.yTop = sector.ceil_height;
                submit(std::make_unique<RenderDataWall_t>(r_wall_data));
                continue;
            }

            // Portal Rendering
            auto it = portal_prev_sec.find(wall.id);
            if (it != portal_prev_sec.end()) {
                // The other sector who also has this edge
                const sector_t& prev_sec = *map.get_sector(it->second);
                bool hole_present = prev_sec.is_interior || sector.is_interior;
                F32 other_flor = prev_sec.floor_height;
                F32 other_ceil = prev_sec.ceil_height;

                F32 min_floor = min(sector.floor_height, other_flor);
                F32 max_floor = max(sector.floor_height, other_flor);
                F32 min_ceil  = min(sector.ceil_height, other_ceil);
                F32 max_ceil  = max(sector.ceil_height, other_ceil);
                
                // Both sector is non-hole
                // If hole is present, only face towards the hole sector
                const vec2& floor_look = (max_floor == sector.floor_height) ? prev_sec.center : sector.center;
                const vec2& ceil_look  = (max_ceil == sector.ceil_height) ? sector.center : prev_sec.center;
                const vec2& center_hole = (sector.is_interior) ? sector.center : prev_sec.center;
                
                // If overlapping, then draw the entire rect and continue to the next wall
                if (max_floor >= min_ceil) {
                    r_wall_data.yBottom = min_floor;
                    r_wall_data.yTop = max_ceil;
                    submit(std::make_unique<RenderDataWall_t>(r_wall_data));
                    continue;
                }

                // Floor Wall
                r_wall_data.yBottom    = min_floor;
                r_wall_data.yTop       = max_floor;
                r_wall_data.ref_center = (hole_present) ? center_hole : floor_look;
            #ifdef GZ_BUILD_DEBUG
                r_wall_data.debug_is_floor = true;
                r_wall_data.debug_is_ceil  = false;
            #endif
                if (hole_present) {
                    r_wall_data.flipped = (sector.is_interior) ? 
                        (max_floor == sector.floor_height)
                        :
                        (max_floor == prev_sec.floor_height);
                }
                submit(std::make_unique<RenderDataWall_t>(r_wall_data));
                
                // Ceil Wall
                r_wall_data.yBottom    = min_ceil;
                r_wall_data.yTop       = max_ceil;
                r_wall_data.ref_center = (hole_present) ? center_hole : ceil_look;
            #ifdef GZ_BUILD_DEBUG
                r_wall_data.debug_is_floor = false;
                r_wall_data.debug_is_ceil  = true;
            #endif
                if (hole_present) {
                    r_wall_data.flipped = (sector.is_interior) ? 
                        (min_ceil == sector.ceil_height)
                        :
                        (min_ceil == prev_sec.ceil_height);
                }
                submit(std::make_unique<RenderDataWall_t>(r_wall_data));
                
            }
            else {
                // First sector to mention this edge
                portal_prev_sec.insert({wall.id, sector.id});
            }
        }

        // Render Floor and Ceil, Floor and ceils are guranteed to be opaque objects
        submit(std::make_unique<RenderDataSector_t>([&]{
            RenderDataSector_t d{};
            d.texture_ids[0] = sector.texture_id_floor;
            d.texture_ids[1] = sector.texture_id_ceil;
            d.shader_id      = lit_shader;
            d.floor          = sector.floor_height;
            d.ceil           = sector.ceil_height;
            d.mesh           = map.get_weak_sector_mesh(sector.id);
            d.index_count    = map.get_sector_mesh(sector.id)->ebo->count();

        #ifdef GZ_BUILD_DEBUG
            d.center = sector.center;
        #endif
            return d;
        }()));
    }
}

void Geez::Renderer::submit(std::unique_ptr<IRenderData> data)
{
    if (!data) 
        return;
    
    RenderType type = data->type;
    if (type == R_NONE || type > R_GUI) 
        return;

    render_buckets[static_cast<size_t>(type)].push_back(std::move(data));
}

void Geez::Renderer::flush()
{
    ImGui::Render();
    
    // Stencil rule
    GL(glStencilMask(0xFF));
    GL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));

    GL(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
    GL(glClearStencil(0));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

    if (!context->validate()) {
        return;
    }

    // Map rendering sequence
    // Wall -> Sector -> Decal -> GameObject -> GUI

    context->active_camera->perspective();
    for (int type = 0; type < R_RENDER_TYPE_COUNT; ++type) {

        context->active_camera->perspective();
        glDisable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);

        if (type == R_WALL) {
            GL(glStencilFunc(GL_ALWAYS, 1, 0xFF)); // Write 1 on walls
        }
        else if (type == R_SECTOR) {
            GL(glStencilFunc(GL_ALWAYS, 2, 0xFF)); // Write 2 on sectors
        }
        else if (type == R_DECAL) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            GL(glPolygonOffset(-1.0f, -1.0f)); // Closer to camera
            GL(glStencilMask(0x00));
            GL(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
            // GL(glStencilFunc(GL_EQUAL, 1, 0xFF));  <- this can be found in render_strategy
        } 
        else if (type == R_GAMEOBJECT) {
            GL(glClear(GL_STENCIL_BUFFER_BIT));
            glDisable(GL_STENCIL_TEST);
        }
        else if (type == R_GUI) {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_STENCIL_TEST);
            context->active_camera->orthographic();
        }

        for (auto& data_ptr : render_buckets[type]) {
            strategies[type]->execute(*data_ptr, *context);
        }

        render_buckets[type].clear();
    }

    context->meshes->unbind();
    context->active_camera->perspective();
}

void Geez::Renderer::display_frame()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(context->active_window->handle());
}

/* ====== DEBUG DRAWS ====== */

void Geez::Internal::debug_draw_ray(
    const RenderContext &context, const glm::vec3 &world_position, 
    const glm::vec3 &direction, F32 length, F32 thickness, const Color3f color)
{
    if (!context.meshes->exists("LINE")) {
        GZ_LOG_FORCE(GZ_FAIL, "Cannot draw ray, no line mesh available.");
        return;
    }
    
    GL(glLineWidth(thickness));
    context.meshes->bind("LINE");

    mat4 model(1);
         model =  translate(model, world_position);
         model *= make_rotation_from_direction(direction);
         model =  scale(model, vec3(1.0f, 1.0f, length));

    Shader* unlit = context.resources->get<Shader>(unlit_shader);
    if (unlit) {
        unlit->bind();
        unlit->
             set_uniform<vec3>("u_color", color)
            .set_uniform<mat4>("u_proj",  context.active_camera->projection_matrix())
            .set_uniform<mat4>("u_view",  context.active_camera->view_matrix())
            .set_uniform<mat4>("u_model", model);
    }
    GL(glDrawArrays(GL_LINES, 0, 2));
    context.meshes->unbind();
}
