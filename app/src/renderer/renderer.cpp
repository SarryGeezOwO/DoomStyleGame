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

// Stencil helpers
void stencil_disable() {
    GL(glDisable(GL_STENCIL_TEST));
    GL(glStencilMask(0xFF));
}

void stencil_reset() {
    GL(glClear(GL_STENCIL_BUFFER_BIT));
}

void stencil_read(Geez::I32 ref, Geez::U32 mask = 0xFF) {
    GL(glStencilMask(0x00));
    GL(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
    GL(glStencilFunc(GL_EQUAL, ref, mask));
}

void stencil_write(Geez::I32 ref, Geez::U32 mask = 0xFF) {
    GL(glStencilMask(0xFF));
    GL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
    GL(glStencilFunc(GL_ALWAYS, ref, mask));
}

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
    strategies[R_WALL]         = std::make_unique<RenderStrategyWall>();
    strategies[R_SECTOR]       = std::make_unique<RenderStrategySector>();
    strategies[R_GAMEOBJECT]   = std::make_unique<RenderStrategyGameobject>();
    strategies[R_GUI]          = std::make_unique<RenderStrategyGUI>();

    // Decal surface types
    strategies_decal[RD_WALL]   = std::make_unique<RenderStrategyDecalWall>();
    strategies_decal[RD_SECTOR] = std::make_unique<RenderStrategyDecalSector>();
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
                RenderDataDecal_t d((decal.target == decal_t::WALL) ? RD_WALL : RD_SECTOR);
                d.position    = decal.position;
                d.normal      = decal.normal;
                d.size        = decal.size;
                d.texture_id  = decal.texture_id;
                d.shader_id   = decal_shader;
                d.target_id   = decal.target_id;
                return d;
            }();
        submit_decal(std::make_unique<RenderDataDecal_t>(r_decal_data));
    }

    // Map geometry or something
    for (const sector_t& sector : map.get_sectors()) {

        for (const U32 wall_id : sector.walls) {
            const wall_t& wall = *map.get_wall(wall_id);
            vec2 point_a = {wall.point_a[0], wall.point_a[1]};
            vec2 point_b = {wall.point_b[0], wall.point_b[1]};

            auto r_wall_data = [&]{
                RenderDataWall_t d{};
                d.id             = wall_id;
                d.shader_id      = lit_shader;
                d.texture_ids[0] = wall.texture_id;
                d.normal         = wall.normal_front();
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
                r_wall_data.yTop    = sector.ceil_height;
                submit(std::make_unique<RenderDataWall_t>(r_wall_data));
                continue;
            }
            else {
                // Portal Rendering
                const sector_t *sa = map.get_sector(wall.connected_sectors[0]);
                const sector_t *sb = map.get_sector(wall.connected_sectors[1]);

                if (sa && sb) { 
                    F32 hf = max(sa->floor_height, sb->floor_height);
                    F32 lf = min(sa->floor_height, sb->floor_height);
                    F32 lc = min(sa->ceil_height,  sb->ceil_height);
                    F32 hc = max(sa->ceil_height,  sb->ceil_height);

                    // If overlapping, then draw the entire rect and continue to the next wall
                    if (hf >= lc) {
                        r_wall_data.yBottom = lf;
                        r_wall_data.yTop    = hc;
                        submit(std::make_unique<RenderDataWall_t>(r_wall_data));
                        continue;
                    }

                    // Floor Wall
                    r_wall_data.yBottom    = lf;
                    r_wall_data.yTop       = hf;
                    r_wall_data.normal     = wall.normal_front();
                    #ifdef GZ_BUILD_DEBUG
                        r_wall_data.debug_is_floor = true;
                        r_wall_data.debug_is_ceil  = false;
                    #endif
                    submit(std::make_unique<RenderDataWall_t>(r_wall_data));   
                
                    // Ceil Wall
                    r_wall_data.yBottom    = lc;
                    r_wall_data.yTop       = hc;
                    r_wall_data.normal     = wall.normal_back();
                    #ifdef GZ_BUILD_DEBUG
                        r_wall_data.debug_is_floor = false;
                        r_wall_data.debug_is_ceil  = true;
                    #endif
                    submit(std::make_unique<RenderDataWall_t>(r_wall_data));
                }
            }
        }

        // Render Floor and Ceil, Floor and ceils are guranteed to be opaque objects
        submit(std::make_unique<RenderDataSector_t>([&]{
            RenderDataSector_t d{};
            d.id             = sector.id;
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
    if (type == R_NONE || type >= R_RENDER_TYPE_COUNT) 
        return;

    render_buckets[static_cast<size_t>(type)].push_back(std::move(data));
}

void Geez::Renderer::submit_decal(std::unique_ptr<IRenderDecalData> data)
{
    if (!data) 
        return;
    
    RenderDecalType type = data->type;
    if (type == RD_NONE || type >= RD_RENDERDECAL_TYPE_COUNT) 
        return;

    decal_buckets[static_cast<size_t>(type)].push_back(std::move(data));   
}

void Geez::Renderer::flush()
{
    ImGui::Render();

    // Stencil rule
    GL(glStencilMask(0xFF));
    GL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
    GL(glStencilFunc(GL_ALWAYS, 1, 0xFF));

    GL(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
    GL(glClearStencil(0));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

    if (!context->validate()) {
        return;
    }

    // Map rendering sequence
    // Wall -> Decal(W) -> Sector -> Decal(S) -> GameObject -> GUI

    context->active_camera->perspective();
    for (int type = 0; type < R_RENDER_TYPE_COUNT; ++type) {

        context->active_camera->perspective();
        glDisable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);

        if (type == R_WALL) {
            // ...
        }
        else if (type == R_SECTOR) {
            decal_buckets[RD_WALL].clear();
            // ...
        }
        else if (type == R_GAMEOBJECT) {
            decal_buckets[RD_SECTOR].clear();
            stencil_reset();
            stencil_disable();
        }
        else if (type == R_GUI) {
            stencil_disable();
            glDisable(GL_DEPTH_TEST);
            context->active_camera->orthographic();
        }

        for (auto& data_ptr : render_buckets[type]) {
            strategies[type]->execute(*data_ptr, *context);

            // render decals attached to surfaces their stencil values
            if (type == R_WALL) {
                GL(glEnable(GL_POLYGON_OFFSET_FILL));
                GL(glPolygonOffset(-1.0f, -1.0f)); // Closer to camera
                stencil_read(1);

                for (auto& decal : decal_buckets[RD_WALL]) {
                    if (decal->target_id == static_cast<RenderDataWall_t&>(*data_ptr).id) {
                        strategies_decal[RD_WALL]->execute(*decal, *context);
                    }
                }

                GL(glDisable(GL_POLYGON_OFFSET_FILL));
                stencil_write(1);
                stencil_reset();
            }
            else if (type == R_SECTOR) {
                GL(glEnable(GL_POLYGON_OFFSET_FILL));
                GL(glPolygonOffset(-1.0f, -1.0f)); // Closer to camera
                stencil_read(2);

                for (auto& decal : decal_buckets[RD_SECTOR]) {
                    if (decal->target_id == static_cast<RenderDataSector_t&>(*data_ptr).id) {
                        strategies_decal[RD_SECTOR]->execute(*decal, *context);
                    } 
                }
                
                GL(glDisable(GL_POLYGON_OFFSET_FILL));
                stencil_write(2);
                stencil_reset();
            }
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
