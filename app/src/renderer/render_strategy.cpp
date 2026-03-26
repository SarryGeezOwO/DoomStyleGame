#include "render_strategy.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "resource/resource.hpp"
#include "util/error.hpp"
#include "util/log.hpp"
#include <gl/glew.h>
#include <glm/glm.hpp>

using namespace glm;

namespace Geez
{
    // Errmm, my mind is currently fried...
    static void bind(const RenderContext &context, const mat4& model,
        const ResourceID& shader_id, const ResourceID& texture_id, bool isFliped, const vec2& uv_scale) 
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
                set_uniform<mat4>("u_proj",  context.active_camera->projection_matrix())
                .set_uniform<mat4>("u_view",  context.active_camera->view_matrix())
                .set_uniform<mat4>("u_model", model);

            if (shader->has_uniform("u_texture"))    shader->set_uniform<I32>("u_texture", 0);
            if (shader->has_uniform("u_uv_scale"))   shader->set_uniform<vec2>("u_uv_scale", uv_scale);
            if (shader->has_uniform("u_normalFlip")) shader->set_uniform<F32>("u_normalFlip", (isFliped ? -1.0f : 1.0f));
        }
    }
}

void Geez::RenderWallStrategy::execute(IRenderData &data, const RenderContext &context)
{
    RenderWallData_t& wall = static_cast<RenderWallData_t&>(data);
    if (!context.meshes->exists("QUAD")) {
        GZ_LOG_FORCE(GZ_FAIL, "Cannot draw wall, no quad mesh available.");
        return;
    }

    Mesh* mesh = context.meshes->get("QUAD");
    vec2 v = wall.b - wall.a;
    vec2 dir  = normalize(v);
    vec2 mid  = wall.a + (v * 0.5f);
    vec2 norm = vec2(-dir.y, dir.x) * (wall.flipped ? -1.0f : 1.0f);
    vec2 to_center = normalize(wall.ref_center - mid);
    
    F32 mag    = abs(length(v));
    F32 angle  = SDL_atan2f(dir.y, dir.x);
    F32 facing = dot(norm, to_center);
    F32 height = abs(wall.yBottom - wall.yTop);

    mesh->bind();
    mat4 model =  mat4(1.0f);
         model =  translate(model, vec3(mid.x, wall.yBottom + (height * 0.5f), mid.y));
         model =  rotate(model, -angle, vec3(0,1,0));
         model =  scale(model, vec3(mag, height, 1.0f));

    bind(
        context, 
        model,
        wall.shader_id, 
        wall.texture_id, 
        (facing < 0.0f), 
        vec2(mag, height)
    );
    GL(glDrawElements(mesh->draw_mode, mesh->index_count(), GL_UNSIGNED_INT, nullptr));
    context.meshes->unbind();

    if (wall.debug_line) {
        vec3 line_center = vec3(mid.x, wall.yBottom + (height*0.5f), mid.y);
        vec3 line_dir = vec3(dir.y * -sign(facing), 0, dir.x * sign(facing));
        GZ_DEBUG_DRAW_RAY(context, line_center, line_dir, 0.075f, 3, Color3f(1,1,0));
    }
}

void Geez::RenderSectorStrategy::execute(IRenderData &data, const RenderContext &context)
{
    RenderSectorData_t& sector = static_cast<RenderSectorData_t&>(data);
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
    bind(
        context, 
        flor_model,
        sector.shader_id, 
        sector.texture_id_flor, 
        false, 
        vec2(1)
    );
    GL(glDrawElements(GL_TRIANGLES, sector.index_count, GL_UNSIGNED_INT, nullptr));
    
    // Ceil Mesh
    mat4 ceil_model = mat4(1.0f);
    ceil_model = translate(ceil_model, vec3(0, sector.ceil, 0));
    bind(
        context, 
        ceil_model, 
        sector.shader_id, 
        sector.texture_id_ceil, 
        true, 
        vec2(1)
    );
    GL(glDrawElements(GL_TRIANGLES, sector.index_count, GL_UNSIGNED_INT, nullptr));

    context.meshes->unbind();
}
