#ifndef GZ_PHYSICS_COMP_HPP
#define GZ_PHYSICS_COMP_HPP

#include <glm/glm.hpp>

namespace Geez
{
    struct GameObject;

    struct physics_component_t
    {
        GameObject* owner = nullptr;

        glm::vec3 position{};
        glm::vec3 velocity{};
        F32 collision_radius;
        F32 step_height;
        F32 height;
        bool grounded;

        physics_component_t() = delete;
        physics_component_t(GameObject* _owner) : owner(_owner) {}
    };
}

#endif