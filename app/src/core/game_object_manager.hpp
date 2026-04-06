#ifndef GZ_GAME_OBJECT_MANAGER_HPP
#define GZ_GAME_OBJECT_MANAGER_HPP

#include "game_object.hpp"
#include "resource/resource.hpp"
#include "util/common_types.hpp"
#include <unordered_map>
#include <memory>

namespace Geez
{
    /**
     * @brief Manages the lifecycle and storage of GameObject instances
     * 
     * GameObjectManager provides functionality to create, destroy, find and retrieve
     * game objects using unique instance IDs. It maintains ownership of all game
     * objects through unique pointers.
     */
    struct GameObjectManager
    {
    private:
        std::unordered_map<InstanceID, std::unique_ptr<GameObject>> m_game_objects;
        
    public:
        void attach_physics_component(const InstanceID& id);

        /**
         * @brief Creates a new basic GameObject instance
         * @param id The unique identifier for the GameObject
         * @return Pointer to the newly created GameObject, nullptr if creation fails
         */
        GameObject* create(const InstanceID& id);

        /**
         * @brief Creates a new GameObject with specified shader
         * @param id The unique identifier for the GameObject
         * @param shader The resource identifier for the shader
         * @return Pointer to the newly created GameObject
         */
        GameObject* create(const InstanceID& id, const ResourceID& shader);

        /**
         * @brief Creates a new GameObject with shader and texture
         * @param id The unique identifier for the GameObject
         * @param shader The resource identifier for the shader
         * @param texture The resource identifier for the texture
         * @return Pointer to the newly created GameObject
         */
        GameObject* create(const InstanceID& id, const ResourceID& shader, const ResourceID& texture);

        /**
         * @brief Removes and deallocates a GameObject
         * @param id The instance ID of the GameObject to destroy
         */
        void destroy(const InstanceID& id);
        void destroy_all();

        /**
         * @brief Checks if a GameObject exists
         * @param id The instance ID to search for
         * @return true if the GameObject exists, false otherwise
         */
        bool find(const InstanceID& id);

        /**
         * @brief Retrieves a GameObject by its ID
         * @param id The instance ID of the desired GameObject
         * @return Pointer to the GameObject if found, nullptr otherwise
         */
        GameObject* get(const InstanceID& id);

        // ===== Iterator API (yields GameObject*) =====
        class Iterator
        {
            using MapIter = std::unordered_map<InstanceID, std::unique_ptr<GameObject>>::iterator;
            MapIter m_it;
        public:
            explicit Iterator(MapIter it) : m_it(it) {}
            GameObject* operator*() const { return m_it->second.get(); }
            GameObject* operator->() const { return m_it->second.get(); }
            Iterator& operator++() { ++m_it; return *this; }
            Iterator operator++(int) { Iterator tmp = *this; ++m_it; return tmp; }
            bool operator==(const Iterator& other) const { return m_it == other.m_it; }
            bool operator!=(const Iterator& other) const { return m_it != other.m_it; }
        };

        class ConstIterator
        {
            using MapConstIter = std::unordered_map<InstanceID, std::unique_ptr<GameObject>>::const_iterator;
            MapConstIter m_it;
        public:
            explicit ConstIterator(MapConstIter it) : m_it(it) {}
            const GameObject* operator*() const { return m_it->second.get(); }
            const GameObject* operator->() const { return m_it->second.get(); }
            ConstIterator& operator++() { ++m_it; return *this; }
            ConstIterator operator++(int) { ConstIterator tmp = *this; ++m_it; return tmp; }
            bool operator==(const ConstIterator& other) const { return m_it == other.m_it; }
            bool operator!=(const ConstIterator& other) const { return m_it != other.m_it; }
        };

        Iterator begin() { return Iterator{m_game_objects.begin()}; }
        Iterator end()   { return Iterator{m_game_objects.end()}; }
        ConstIterator begin() const { return ConstIterator{m_game_objects.cbegin()}; }
        ConstIterator end()   const { return ConstIterator{m_game_objects.cend()}; }
        ConstIterator cbegin() const { return ConstIterator{m_game_objects.cbegin()}; }
        ConstIterator cend()   const { return ConstIterator{m_game_objects.cend()}; }
    };

}

#endif