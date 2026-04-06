#ifndef GZ_PHYSICS_HPP
#define GZ_PHYSICS_HPP

#include "core/game_object_manager.hpp"
#include "gmp/gmp.hpp"
#include "physics/physics_comp.hpp"
#include "util/common_types.hpp"
#include <glm/glm.hpp>

namespace Geez
{
    struct PhysicsSystem {
    private:
        void apply_gravity(physics_component_t& obj, F32 dt) const;
        void vertical_collision(physics_component_t& obj,   const GeezMapData& map) const;
        void horizontal_collision(physics_component_t& obj, const GeezMapData& map) const;
        void apply_velocity(physics_component_t& obj, F32 dt) const;

    public:
        F32 gravity;
        void update(GameObjectManager& manager, const GeezMapData& map, float dt);
    };
}

#endif