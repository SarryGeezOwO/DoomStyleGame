#include "physics.hpp"
#include "util/geometry_util.hpp"

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
            obj.position.y += gravity * delta_time;
        }
    }

    void PhysicsSystem::vertical_collision(physics_component_t &obj, const GeezMapData &map) const
    {
        obj.grounded = false;
        const F32 groundY   = (obj.position.y - (obj.height * 0.5f));
        const vec2 posXZ    = vec2(obj.position.x, obj.position.z);
        F32 bestFloor       = -1000000.0f;
        bool foundSector    = false;

        // =============== Vertical collision SCOPE =============== //
        for (const sector_t& sector : map.get_sectors()) {
            bool isInside  = is_point_in_sector(posXZ, sector);
            if (!isInside) continue;

            // Get the highest floor sector on your standig point
            if (sector.floor_height > bestFloor) {
                bestFloor   = sector.floor_height;
                foundSector = true;
            }
        }

        if (foundSector) {
            if (groundY <= bestFloor) {
                obj.position.y = (bestFloor + (obj.height * 0.5f));
                obj.velocity.y = 0;
                obj.grounded = true;
            }
        }
    }

    void PhysicsSystem::horizontal_collision(physics_component_t &obj, const GeezMapData &map) const
    {
        const vec2 posXZ    = vec2(obj.position.x, obj.position.z);

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
                        const F32 highest_floor = max(sa->floor_height, sb->floor_height);
                        const F32 lowest_ceil   = min(sa->ceil_height,  sb->ceil_height);
                        const F32 gap = abs(highest_floor - lowest_ceil); 

                        // Bypass collision checking upon these conditions
                        // Gap between max_floor and min_ceil is greater than or equal to the  object_height 
                        if (gap >= obj.height)
                        {
                            if (highest_floor < obj.position.y && sector.floor_height < lowest_ceil) 
                                continue; // Stepping down
                            else if (sfDiff <= obj.step_height)
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
                }
            }   
        }
    }

    void PhysicsSystem::apply_velocity(physics_component_t &obj) const
    {
        obj.position += obj.velocity * delta_time;
    }

    void PhysicsSystem::update(GameObjectManager &manager, const GeezMapData &map, float dt) {

        delta_time = dt;
        for (GameObject* obj : manager) {
            if (!obj->physics) continue;
            obj->physics->position = obj->position;
            apply_gravity(*obj->physics);
            vertical_collision(*obj->physics, map);
            apply_velocity(*obj->physics);
            horizontal_collision(*obj->physics, map);
            obj->position = obj->physics->position;
        }
    }
}
