#ifndef GZ_GAME_OBJECT_HPP
#define GZ_GAME_OBJECT_HPP

#include "util/common_types.hpp"
#include "resource/resource.hpp"
#include "physics/physics_comp.hpp"

#include <glm/glm.hpp>
#include <string>

namespace Geez
{
    using InstanceID = std::string;

    struct GameObject
    {
        friend struct GameObjectManager;

    private:
        InstanceID m_id;

    public:
        physics_component_t* physics = nullptr; // instantiated with 'new'
        
        glm::vec3 position;
        glm::vec3 scale;
        
        ResourceID texture_id;
        ResourceID shader_id;

        GameObject() = delete; 
        
        GameObject(const InstanceID& name) 
            : m_id(name), position(0), scale(1.0f) {}
        
        ~GameObject() {
            delete physics;
        }

        inline InstanceID id() const noexcept { return m_id; }
    };
}

#endif