#include "render_strategy.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "resource/resource.hpp"
#include "util/geometry_util.hpp"
#include "util/error.hpp"
#include "util/log.hpp"
#include <gl/glew.h>
#include <glm/glm.hpp>

using namespace glm;

namespace Geez
{
    // Errmm, my mind is currently fried...
    static void bind(const RenderContext &context, const mat4& model,
        const ResourceID& shader_id, const ResourceID& texture_id, 
        bool isFliped, bool use_uv_world, const vec2& uv_scale = vec2(1))
    {

        // Pass in nullptr on texture for no texture duh...
        Shader*  shader  = context.resources->get<Shader>(shader_id);
        Texture* texture = context.resources->get<Texture>(texture_id);

        if (texture) {
            texture->bind(0);   
        }

        if (shader) {
            shader->bind();
            shader->
                set_uniform<mat4>("u_proj",   context.active_camera->projection_matrix())
                .set_uniform<mat4>("u_view",  (context.active_camera->is_orthographic() ? mat4(1.0f) : context.active_camera->view_matrix()))
                .set_uniform<mat4>("u_model", model);

            if (shader->has_uniform("u_texture"))     shader->set_uniform<I32>("u_texture", 0);
            if (shader->has_uniform("u_uv_scale"))    shader->set_uniform<vec2>("u_uv_scale", uv_scale);
            if (shader->has_uniform("u_use_world_uv"))shader->set_uniform<bool>("u_use_world_uv", use_uv_world);
            if (shader->has_uniform("u_normalFlip"))  shader->set_uniform<F32>("u_normalFlip", (isFliped ? -1.0f : 1.0f));
        }
    }
}

void Geez::RenderStrategyWall::execute(IRenderData &data, const RenderContext &context)
{
    const RenderDataWall_t& wall = static_cast<RenderDataWall_t&>(data);
    if (!context.meshes->exists("QUAD")) {
        GZ_LOG_FORCE(GZ_FAIL, "Cannot draw wall, no quad mesh available.");
        return;
    }

    Mesh* mesh = context.meshes->get("QUAD");
    const vec2 v = wall.b - wall.a;
    const vec2 dir  = normalize(v);
    const vec2 mid  = wall.a + (v * 0.5f);
    const vec2 norm = vec2(-dir.y, dir.x) * (wall.flipped ? -1.0f : 1.0f);
    const vec2 to_center = normalize(wall.ref_center - mid);
    
    const F32 mag    = abs(length(v));
    const F32 angle  = SDL_atan2f(dir.y, dir.x);
    const F32 facing = dot(norm, to_center);
    const F32 height = abs(wall.yBottom - wall.yTop);

    mesh->bind();
    mat4 model =  mat4(1.0f);
         model =  translate(model, vec3(mid.x, wall.yBottom + (height * 0.5f), mid.y));
         model =  rotate(model, -angle, vec3(0,1,0));
         model =  scale(model, vec3(mag, height, 1.0f));

    bind(
        context, 
        model,
        wall.shader_id, 
        wall.texture_ids[0], 
        (facing < 0.0f),
        true,
        vec2(1.5)
    );
    GL(glDrawElements(mesh->draw_mode, mesh->index_count(), GL_UNSIGNED_INT, nullptr));
    context.meshes->unbind();

    #ifdef GZ_BUILD_DEBUG
        const vec3 line_center = vec3(mid.x, wall.yBottom + (height*0.5f), mid.y);
        const vec3 line_dir = vec3(dir.y * -sign(facing), 0, dir.x * sign(facing));
        const Color3f color = Color3f(wall.debug_is_ceil, 0, wall.debug_is_floor);
        GZ_DEBUG_DRAW_RAY(context, line_center, line_dir, 0.075f, 3, color);
    #endif
}

void Geez::RenderStrategySector::execute(IRenderData &data, const RenderContext &context)
{
    const RenderDataSector_t& sector = static_cast<RenderDataSector_t&>(data);
    if (sector.mesh.expired()) {
        GZ_LOG_FORCE(GZ_FAIL, "[RENDERER] Cannot render sector, pointer expired.");
        return; 
    }

    std::shared_ptr<sector_mesh_t> sector_mesh = sector.mesh.lock();
    if (!sector_mesh) {
        GZ_LOG_FORCE(GZ_FAIL, "[RENDERER] Cannot render sector, missing mesh.");
        return;
    }

    sector_mesh->vao->bind();

    // Floor Mesh
    mat4 flor_model = mat4(1.0f);
    flor_model = translate(flor_model, vec3(0, sector.floor, 0));
    const vec2 uv_scale(1.5);

    bind(
        context, 
        flor_model,
        sector.shader_id, 
        sector.texture_ids[0], 
        false, 
        false,
        uv_scale
    );
    GL(glDrawElements(GL_TRIANGLES, sector.index_count, GL_UNSIGNED_INT, nullptr));
    
    // Ceil Mesh
    mat4 ceil_model = mat4(1.0f);
    ceil_model = translate(ceil_model, vec3(0, sector.ceil, 0));
    bind(
        context, 
        ceil_model, 
        sector.shader_id, 
        sector.texture_ids[1], 
        true, 
        false,
        uv_scale
    );
    GL(glDrawElements(GL_TRIANGLES, sector.index_count, GL_UNSIGNED_INT, nullptr));
    context.meshes->unbind();

    #ifdef GZ_BUILD_DEBUG
        const vec3 WORLD_UP = vec3(0,1,0);
        const vec3 floorC = vec3(sector.center.x, sector.floor, sector.center.y);
        const vec3 ceilC  = vec3(sector.center.x, sector.ceil,  sector.center.y);
        GZ_DEBUG_DRAW_RAY(context, floorC,  WORLD_UP, 0.075f, 3);
        GZ_DEBUG_DRAW_RAY(context, ceilC,  -WORLD_UP, 0.075f, 3);
    #endif
}

void Geez::RenderStrategyDecal::execute(IRenderData &data, const RenderContext &context)
{
    const RenderDataDecal_t& decal = static_cast<RenderDataDecal_t&>(data);
    if (!context.meshes->exists("DECAL")) {
        GZ_LOG_FORCE(GZ_FAIL, "Cannot draw decal, no decal mesh available.");
        return;
    }

    Mesh* mesh = context.meshes->get("DECAL");
    mat4 model = mat4(1.0f);
        model  = translate(model, decal.position + decal.normal * 0.01f);
        model *= make_rotation_from_direction(decal.normal);
        model  = scale(model, vec3(decal.size.x, decal.size.y, 1.0f));

    mesh->bind();
    bind(
        context,
        model,
        decal.shader_id,
        decal.texture_ids[0],
        false,
        false
    );
    GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr)); // Magic 6
    context.meshes->unbind();
}

void Geez::RenderStrategyGameobject::execute(IRenderData &data, const RenderContext &context)
{
    const RenderDataGameobject_t& object = static_cast<RenderDataGameobject_t&>(data);
    if (!context.meshes->exists("QUAD")) {
        GZ_LOG_FORCE(GZ_FAIL, "Cannot draw billboard, no quad mesh available.");
        return;
    }

    mat4 camRot = mat4(1.0f);
    camRot[0] = vec4( context.active_camera->axis_right(),    0);
    camRot[1] = vec4( vec3(0, 1, 0),                          0);
    camRot[2] = vec4(-context.active_camera->axis_forward(),  0);

    Mesh* mesh = context.meshes->get("QUAD");
    mat4 model =  mat4(1.0f);
        model =  translate(model, object.position);
        model *= camRot;
        model =  scale(model, object.scale);

    mesh->bind();
    bind(
        context,
        model,
        object.shader_id,
        object.texture_ids[0],
        false,
        false
    );
    GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr)); // Magic 6
    context.meshes->unbind();
}

void Geez::RenderStrategyGUI::execute(IRenderData &data, const RenderContext &context)
{
    const RenderDataGUI_t& gui = static_cast<RenderDataGUI_t&>(data);
    if (!context.meshes->exists("QUAD")) {
        GZ_LOG_FORCE(GZ_FAIL, "Cannot draw billboard, no quad mesh available.");
        return;
    }

    Mesh* mesh = context.meshes->get("QUAD");
    mat4 model =  mat4(1.0f);
        model =  translate(model, vec3(gui.screen_pos.x, gui.screen_pos.y, 0.0f));
        model =  rotate(model, radians(gui.angle), vec3(0, 0, 1));
        model =  scale(model, vec3(gui.size.x, gui.size.y, 1));

    mesh->bind();
    bind(
        context,
        model,
        gui.shader_id,
        gui.texture_ids[0],
        false,
        false
    );
    GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr)); // Magic 6  
    context.meshes->unbind();
}