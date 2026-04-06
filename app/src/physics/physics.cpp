#include "physics.hpp"
#include "util/geometry_util.hpp"
#include "util/utility.hpp"

using namespace glm;
namespace Geez
{
    static F32 delta_time;

    /*
        Object position is located at the center of the model
        x -> width/2
        y -> height/2
        kind of thing...
    */

    // position if velocity is applied
    void PhysicsSystem::apply_gravity(physics_component_t &obj) const
    {
        if (!obj.grounded) {
            obj.force.y += gravity * obj.mass;
        }
    }

    void PhysicsSystem::integrate_force(physics_component_t &obj) const
    {
        if (obj.mass <= 0.0f) return;

        vec3 acceleration = obj.force / obj.mass;
        obj.velocity += acceleration * delta_time;
        obj.force = vec3(0.0f);
    }

    void PhysicsSystem::vertical_collision(physics_component_t &obj, const GeezMapData &map) const
    {
        obj.grounded = false;
        const F32 botY   = (obj.position.y - (obj.height * 0.5f));
        const F32 topY   = (obj.position.y + (obj.height * 0.5f));
        const vec2 posXZ = vec2(obj.position.x, obj.position.z);
        const sector_t *bestSector = nullptr;

        // =============== Vertical collision SCOPE =============== //
        for (const sector_t& sector : map.get_sectors()) {
            bool isInside  = is_point_in_sector(posXZ, sector);
            if (!isInside) continue;

            // Get the highest floor sector on your standig point
            // And get that ceil as well
            if (bestSector) {
                if (sector.floor_height > bestSector->floor_height)
                    bestSector = &sector;
            }
            else bestSector = &sector;
        }

        if (bestSector) {

            // Floor
            if (botY - 0.01f <= bestSector->floor_height) {
                obj.position.y = (bestSector->floor_height + (obj.height * 0.5f));
                obj.velocity.y = 0;
                obj.grounded = true;
            }
            else if ((topY + obj.velocity.y * delta_time) > bestSector->ceil_height) {
                obj.position.y = (bestSector->ceil_height - (obj.height * 0.5f));
                obj.velocity.y = 0;
            }
        }
    }

    void PhysicsSystem::horizontal_collision(physics_component_t &obj, const GeezMapData &map) const
    {
        const vec2 posXZ    = vec2(obj.position.x, obj.position.z);
        const F32  posY     = obj.position.y;

        for (const sector_t& sector : map.get_sectors()) {
            bool isCameraInSector = is_point_in_sector(posXZ, sector);
            if (!isCameraInSector) continue; 

            for (U32 wid : sector.walls) {
                const wall_t& wall = *map.get_wall(wid);

                // Since walls absolutely only contains two sectors as "connected"
                // compare their heights, and only allow pass through when below threshold
                // pass if max floor is less than realCamY
                if (wall.is_portal) {
                    const sector_t *sa = map.get_sector(wall.connected_sectors[0]);
                    const sector_t *sb = map.get_sector(wall.connected_sectors[1]);

                    if (sa && sb) { 
                        const F32 sfDiff = abs(sa->floor_height - sb->floor_height);
                        const F32 hf     = max(sa->floor_height, sb->floor_height);
                        const F32 lf     = min(sa->floor_height, sb->floor_height);
                        const F32 lc     = min(sa->ceil_height,  sb->ceil_height);
                        const F32 hc     = max(sa->ceil_height,  sb->ceil_height);
                        const F32 gap    = abs(hf - lc); 
                        const bool wall_check_f = !number_in_range(posY - ((obj.height * 0.5f) - 0.02f), lf, hf);
                        const bool wall_check_c = !number_in_range(posY + ((obj.height * 0.5f) - 0.02f), lc, hc);

                        // Bypass collision checking upon these conditions
                        // Gap between max_floor and min_ceil is greater than or equal to the  object_height 
                        if (gap >= obj.height)
                        {
                            if (hf <= posY && wall_check_c) 
                                continue; // Stepping down
                            else if (sfDiff <= obj.step_height && wall_check_f)
                                continue; // stepping up scenario
                        }
                    }
                }

                const vec2 p1(wall.point_a[0], wall.point_a[1]);
                const vec2 p2(wall.point_b[0], wall.point_b[1]);

                vec2 closest = closest_point_on_segment(p1, p2, posXZ);
                vec2 diff    = posXZ - closest;
                F32 dist     = length(diff);

                if (dist < obj.collision_radius) {
                    vec2 pushDir = normalize(diff);
                    F32 penetration = obj.collision_radius - dist;
                    obj.position.x += pushDir.x * penetration;
                    obj.position.z += pushDir.y * penetration;
                    obj.velocity.x = 0;
                    obj.velocity.z = 0;
                }
            }   
        }
    }

    void PhysicsSystem::apply_velocity(physics_component_t &obj) const
    {
        obj.position += obj.velocity * delta_time;
    }

    void PhysicsSystem::apply_friction(physics_component_t &obj) const
    {
        if (!obj.grounded) return;
        const F32 f = (1.0f - clamp(obj.friction, 0.0f, 1.0f));
        obj.velocity.x *= f;
        obj.velocity.z *= f;
    }

    void PhysicsSystem::update(GameObjectManager &manager, const GeezMapData &map, float dt) {

        delta_time = dt;
        for (GameObject* obj : manager) {
            physics_component_t* p_comp = obj->physics;

            if (!p_comp) continue;
            p_comp->position = obj->position;
            apply_gravity       (*p_comp);
            integrate_force     (*p_comp);
            vertical_collision  (*p_comp, map);
            apply_velocity      (*p_comp);
            apply_friction      (*p_comp);
            horizontal_collision(*p_comp, map);
            obj->position = p_comp->position;
        }
    }

    void PhysicsSystem::add_force(GameObjectManager &manager, const InstanceID &id, const vec3 &force)
    {
        if (manager.get(id)->physics) {
            manager.get(id)->physics->force += force;
        }
    }

    void PhysicsSystem::add_impulse_force(GameObjectManager &manager, const InstanceID &id, const vec3 &force)
    {
        physics_component_t* p = manager.get(id)->physics;
        if (p) {
            p->velocity += force / p->mass;
        }
    }
}
