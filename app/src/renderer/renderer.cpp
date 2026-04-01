#include <renderer.hpp>
#include <SDL3/SDL.h>

#include "util/error.hpp"
#include "util/log.hpp"
#include "util/geometry_util.hpp"
#include <gl/glew.h>
#include <algorithm>

using namespace glm;

// TEMP
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

    if (!gameobjects) {
        GZ_LOG_FORCE(GZ_FAIL, "[RENDERER] RenderContext missing gameobjects.");
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
        gameobjects     &&
        meshes          &&
        resources;
}


Geez::Renderer::Renderer()
{
    // Load Strategies here
    strategies[R_WALL]       = std::make_unique<RenderStrategyWall>();
    strategies[R_SECTOR]     = std::make_unique<RenderStrategySector>();
    strategies[R_GAMEOBJECT] = std::make_unique<RenderStrategyGameobject>();
    strategies[R_GUI]        = std::make_unique<RenderStrategyGUI>();
}

Geez::Renderer::~Renderer()
{
    GZ_LOG(GZ_DEBUG, "Renderer quit");
}

void Geez::Renderer::submit_map_geometry(GeezMapData &map)
{
    // Wall_ID, Sector_ID
    std::unordered_map<U32, U32> portal_prev_sec;
    const vec2 camPosXZ(context->active_camera->position.x, context->active_camera->position.y);
    
    for (const sector_t& sector : map.get_sectors()) { 
        for (const U32 wall_id : sector.walls) {
            const wall_t& wall = *map.get_wall(wall_id);
            const vec2 point_a = {wall.point_a[0], wall.point_a[1]};
            const vec2 point_b = {wall.point_b[0], wall.point_b[1]};

            RenderDataWall_t wall_data;
            wall_data.debug_line    = true;
            wall_data.flipped       = false;
            wall_data.shader_id     = lit_shader;
            wall_data.texture_id    = wall.texture_id;
            wall_data.ref_center    = sector.center;
            wall_data.a             = point_a;
            wall_data.b             = point_b;

            // Render full quad from ceil to floor
            if (!wall.is_portal) {
                wall_data.yBottom = sector.floor_height;
                wall_data.yTop = sector.ceil_height;
                submit(std::make_unique<RenderDataWall_t>(wall_data));
                continue;
            }

            // Portal Rendering
            auto it = portal_prev_sec.find(wall.id);
            if (it != portal_prev_sec.end()) {
                // The other sector who also has this edge
                const sector_t& prev_sec = *map.get_sector(it->second);
                bool hole_present = prev_sec.is_hole || sector.is_hole;
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
                const vec2& center_hole = (sector.is_hole) ? sector.center : prev_sec.center;
                
                // If overlapping, then draw the entire rect and continue to the next wall
                if (max_floor >= min_ceil) {
                    wall_data.yBottom = min_floor;
                    wall_data.yTop = max_ceil;
                    submit(std::make_unique<RenderDataWall_t>(wall_data));
                    continue;
                }

                // Floor Wall
                wall_data.yBottom    = min_floor;
                wall_data.yTop       = max_floor;
                wall_data.ref_center = (hole_present) ? center_hole : floor_look;
                if (hole_present) {
                    wall_data.flipped = (sector.is_hole) ? 
                        (max_floor == sector.floor_height)
                        :
                        (max_floor == prev_sec.floor_height);
                }
                submit(std::make_unique<RenderDataWall_t>(wall_data));
                
                // Ceil Wall
                wall_data.yBottom    = min_ceil;
                wall_data.yTop       = max_ceil;
                wall_data.ref_center = (hole_present) ? center_hole : ceil_look;
                if (hole_present) {
                    wall_data.flipped = (sector.is_hole) ? 
                        (min_ceil == sector.ceil_height)
                        :
                        (min_ceil == prev_sec.ceil_height);
                }
                submit(std::make_unique<RenderDataWall_t>(wall_data));
                
            }
            else {
                // First sector to mention this edge
                portal_prev_sec.insert({wall.id, sector.id});
            }
        }

        // Render Floor and Ceil, Floor and ceils are guranteed to be opaque objects
        RenderDataSector_t r_sector;
        r_sector.texture_id_flor = sector.texture_id_floor;
        r_sector.texture_id_ceil = sector.texture_id_ceil;
        r_sector.shader_id  = lit_shader;
        r_sector.floor = sector.floor_height;
        r_sector.ceil  = sector.ceil_height;
        r_sector.mesh  = map.get_weak_sector_mesh(sector.id);
        r_sector.index_count = map.get_sector_mesh(sector.id)->ebo->count();
        submit(std::make_unique<RenderDataSector_t>(r_sector));
        
        // Don't ask why this debug_line is in here
        // as opposed to being in render_strategy.cpp
        vec3 WORLD_UP = vec3(0,1,0);
        GZ_DEBUG_DRAW_RAY(*context.get(), vec3(sector.center.x, sector.floor_height, sector.center.y),  WORLD_UP, 0.075f, 3);
        GZ_DEBUG_DRAW_RAY(*context.get(), vec3(sector.center.x, sector.ceil_height,  sector.center.y), -WORLD_UP, 0.075f, 3);
    }
}

void Geez::Renderer::submit(std::unique_ptr<IRenderData> data)
{
    render_list.push_back(std::move(data));
}

void Geez::Renderer::flush()
{
    if (!context->validate()) {
        return;
    }

    // Map rendering or something sorted by Render Category
    // Wall -> Sector -> -> GameObject -> GUI
    std::sort(render_list.begin(), render_list.end(), [](const auto& a, const auto& b) 
    { return a->type < b->type; });

    for (const auto& data_ptr : render_list) 
    {   
        IRenderData* data       = data_ptr.get();
        const RenderType type   = data->type;

        if (type == R_NONE) {
            continue;
        }
        
        strategies[type]->execute(*data, *context.get());
    }
    render_list.clear();

    /*
        TODO:
            Migrate gameobject rendering to main.cpp and by 
            RenderStrategy, Also Crosshair, so we can actually fucking start 
            doing gameplay.
    */

    // Gameobjects
    if (!context->meshes->exists("QUAD")) {
        GZ_LOG_FORCE(GZ_FAIL, "Cannot draw Billboards(Gameobjects), no quad mesh available.");
        SDL_GL_SwapWindow(context->active_window->handle());
        return;
    }

    context->meshes->bind("QUAD");
    for (const GameObject* object : *context->gameobjects) {
        mat4 model =  mat4(1.0f);
             model =  translate(model, object->position);
             model *= make_rotation_from_quaternion(object->rotation);
             model =  scale(model, object->scale);

        Shader*  shader  = context->resources->get<Shader>(object->shader_id);
        Texture* texture = context->resources->get<Texture>(object->texture_id);

        if (texture) texture->bind(0);
        if (shader) {
            shader->bind();
            shader->
                set_uniform<I32>("u_texture", 0)
                .set_uniform<mat4>("u_proj",  context->active_camera->projection_matrix())
                .set_uniform<mat4>("u_view",  context->active_camera->view_matrix())
                .set_uniform<mat4>("u_model", model)
                .set_uniform<vec2>("u_uv_scale", vec2(1))
                .set_uniform<vec2>("u_uv_offset", vec2(0));

            if (shader->has_uniform("u_normalFlip")) {
                shader->set_uniform<F32>("u_normalFlip", 1.0f);
            }
        }
        GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr)); // Magic 6
    }

    context->meshes->unbind();
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
