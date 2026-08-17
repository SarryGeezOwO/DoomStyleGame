#ifndef GZ_GMP_HPP
#define GZ_GMP_HPP

#include "gmp_types.hpp"
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <queue>
#include <functional>

namespace Geez
{
    // TagA, TagB, from (0=none 1=A 2=B 3=Both)
    using TagCallback = std::function<void(UPTR, UPTR, U8)>;

    struct TagResolver {

        // A connection is where any of the participating members action
        // will trigger a callback
        struct TagConnection {
            U32 a;
            U32 b;
            TagCallback cb;
        };

    private:
        std::unordered_map<U32, ITagClient const*> tag_map;
        std::vector<TagConnection> connections;
        
    public:

        TagResolver();
        ~TagResolver();

        inline void addTagClient(ITagClient const *client) {
            tag_map[client->tag_id] = client;
            GZ_LOG(GZ_DEBUG, "Tag Client added %d", client->tag_id);
        }

        inline void addTagConnection(U32 id_a, U32 id_b, TagCallback callback) {
            connections.push_back({id_a, id_b, callback});
            GZ_LOG(GZ_DEBUG, "Tag Connection between %d and %d added", id_a, id_b);
        }
        
        template <typename T>
        T const* base_object(U32 tag_id) {
            if (tag_map.find(tag_id) == tag_map.end()) {
                GZ_LOG(GZ_FAIL, "Tag ID [%d] not found...", tag_id);
                return nullptr;
            }
            return reinterpret_cast<T const*>(tag_map.at(tag_id));            
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

        U32 make_decal(glm::vec3 pos, glm::vec2 size, ResourceID texture_id, decal_t::target_t target, U32 target_id);
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
            static void interact_decal(ITagClient *ptr);
        };
    };
}

#endif