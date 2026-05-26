#ifndef GZ_GMP_HPP
#define GZ_GMP_HPP

#include "gmp_types.hpp"
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <queue>

namespace Geez
{
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
    };
}

#endif