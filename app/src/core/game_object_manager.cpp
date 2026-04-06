#include "game_object_manager.hpp"
#include "util/log.hpp"
#include <glm/glm.hpp>

void Geez::GameObjectManager::attach_physics_component(const InstanceID &id)
{
    GameObject* obj = get(id);
    if (obj) {
        obj->physics = new physics_component_t(obj);

        // Defaults
        obj->physics->collision_radius  = 0.1f;
        obj->physics->step_height       = 0.0f;
        obj->physics->mass              = 1.0f;
        obj->physics->friction          = 0.1f;
    }
    else {
        GZ_LOG(GZ_FAIL, "Cannot attach physics component to nullptr GameObject");
    }
}

Geez::GameObject *Geez::GameObjectManager::create(const InstanceID &id)
{
    if (m_game_objects.find(id) != m_game_objects.end()) {
        GZ_LOG(GZ_FAIL, "GameObject [%s] already existed", id.c_str()); 
        return nullptr;
    } 
    std::unique_ptr<GameObject> instance = std::make_unique<GameObject>(id);
    instance->shader_id = "unlit_texture";
    instance->texture_id = "DefaultTexture";
    instance->visible = true;
    
    m_game_objects.insert({id, std::move(instance)});
    GZ_LOG(GZ_OK, "GameObject Created [%s]", id.c_str());
    return m_game_objects.at(id).get();
}

Geez::GameObject *Geez::GameObjectManager::create(const InstanceID &id, const ResourceID &shader)
{
    GameObject* obj = create(id);
    obj->shader_id = shader;
    return m_game_objects.at(id).get();
}

Geez::GameObject *Geez::GameObjectManager::create(const InstanceID &id, const ResourceID &shader, const ResourceID &texture)
{
    GameObject* obj = create(id);
    obj->shader_id = shader;
    obj->texture_id = texture;
    return m_game_objects.at(id).get();
}

void Geez::GameObjectManager::destroy(const InstanceID &id)
{
    if (m_game_objects.find(id) == m_game_objects.end()) {
        GZ_LOG(GZ_FAIL, "GameObject [%s] doesn't exist", id.c_str()); 
        return;
    } 
    m_game_objects.erase(id);
    GZ_LOG(GZ_OK, "GameObject Destroyed [%s]", id.c_str());
}

void Geez::GameObjectManager::destroy_all()
{
    for (auto obj : *this) {
        destroy(obj->m_id);
    }
}

// Returns true if object exists
bool Geez::GameObjectManager::find(const InstanceID &id)
{
    return m_game_objects.find(id) != m_game_objects.end();
}

Geez::GameObject* Geez::GameObjectManager::get(const InstanceID &id)
{
    if (m_game_objects.find(id) == m_game_objects.end()) {
        GZ_LOG(GZ_FAIL, "GameObject [%s] doesn't exist", id.c_str()); 
        return nullptr;
    } 
    return m_game_objects.at(id).get(); 
}
