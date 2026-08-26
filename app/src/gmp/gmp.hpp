#ifndef GZ_GMP_HPP
#define GZ_GMP_HPP

#include "gmp_types.hpp"
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <queue>
#include <functional>

namespace Geez
{
    // TagA, TagB, from (0=none 1=A 2=B 3=Both)
    using TagCallback = std::function<void(UPTR, UPTR, U8)>;

    template <typename Derived>
    inline Derived* tag_ptr_cast(UPTR ptr)
    {
        static_assert(std::is_base_of<ITagClient, Derived>::value,
            "tag_ptr_cast: Derived must inherit from ITagClient");

        ITagClient* base = reinterpret_cast<ITagClient*>(ptr);
        return static_cast<Derived*>(base);
    }

    struct TagResolver {

        // A connection is where any of the participating members action
        // will trigger a callback
        struct TagConnection {
            U32 a;
            U32 b;
            TagCallback cb;
        };

    private:
        std::unordered_map<U32, ITagClient*> tag_map;
        std::vector<TagConnection> connections;
        
    public:

        TagResolver();
        ~TagResolver();

        inline void addTagClient(ITagClient& client) {
            // Declines tag_ids of 0
            if (client.tag_id == 0)
                return;
            tag_map[client.tag_id] = &client;
        }

        inline void addTagConnection(U32 id_a, U32 id_b, TagCallback callback) {
            if (tag_map.find(id_a) == tag_map.end() || tag_map.find(id_b) == tag_map.end())
                return;
            connections.push_back({id_a, id_b, callback});
        }

        inline const std::vector<U32> get_tags() const noexcept {
            std::vector<U32> keys;
            keys.reserve(tag_map.size());

            for (const auto& pair : tag_map)
                keys.push_back(pair.first);
            return keys;
        }

        inline const std::vector<TagConnection> get_connections() const noexcept {
            return connections;
        }
               
        void resolve_tag(U32 tag_id);
        void resolve_all_tag();
    };



    struct GeezMapData : IResource 
    {
    private:
        std::vector<wall_t>     walls;
        std::vector<sector_t>   sectors;
        std::vector<decal_t>    decals;
        std::unordered_map<U32, std::string> texture_references;
        std::unordered_map<U32, std::shared_ptr<sector_mesh_t>> sector_meshes; // Sector_id

        // Sectors that needs updating, will trigger at main's post_update()
        std::queue<U32> m_update_queue;

    public:
        std::unique_ptr<TagResolver> tag_resolver;
 
        GeezMapData(const std::string& file);
        ~GeezMapData();

        void make_mesh();
        bool is_sector_scary(U32 sector_id, U32 wall_id);

        void update_sectors();
        void update_wall_normal(wall_t& wall);
        void update_all_wall_normal();

        void set_sector_floor(sector_t& sector, F32 new_floor, bool additive = false);
        void set_sector_floor(U32 id, F32 new_floor, bool additive = false);
        void set_sector_ceil(sector_t& sector, F32 new_ceil, bool additive = false);
        void set_sector_ceil(U32 id, F32 new_ceil, bool additive = false);

        // Note: the created decal_t is not registered as a TagClient. You have to register it yourself
        U32 make_decal(glm::vec3 pos, glm::vec2 size, ResourceID texture_id, decal_t::target_t target, U32 target_id, U32 tag_id);
        void update_decal(decal_t* decal, glm::vec3 new_pos, decal_t::target_t target, U32 target_id);
        void update_decal(U32 decal_id, glm::vec3 new_pos, decal_t::target_t target, U32 target_id);

        inline U32 get_wall_count() const { return walls.size(); }
        inline U32 get_sector_count() const { return sectors.size(); }
        inline U32 get_sector_mesh_count() const { return sector_meshes.size(); }

        decal_t* get_decal(U32 decal_id);
        const decal_t* get_decal(U32 decal_id) const; 

        std::vector<decal_t*> get_wall_decals(U32 wall_id);
        std::vector<const decal_t*> get_wall_decals(U32 wall_id) const; 

        std::vector<decal_t*> get_sector_decals(U32 sector_id);
        std::vector<const decal_t*> get_sector_decals(U32 sector_id) const; 

        wall_t* get_wall(U32 wall_id);
        const wall_t* get_wall(U32 wall_id) const;

        sector_t* get_sector(U32 sector_id);
        const sector_t* get_sector(U32 sector_id) const;

        sector_mesh_t* get_sector_mesh(U32 sector_id);
        const sector_mesh_t* get_sector_mesh(U32 sector_id) const;
       
        std::weak_ptr<sector_mesh_t> get_weak_sector_mesh(U32 sector_id);
        std::weak_ptr<const sector_mesh_t> get_weak_sector_mesh(U32 sector_id) const;
        
        inline const std::vector<wall_t>    get_walls()   const { return walls;   }
        inline const std::vector<sector_t>  get_sectors() const { return sectors; }
        inline const std::vector<decal_t>   get_decals()  const { return decals;  }

        struct Event {
            static void interact_decal(ITagClient* ptr);
        };
    };
}

#endif