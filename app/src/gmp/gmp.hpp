#ifndef GZ_GMP_HPP
#define GZ_GMP_HPP

#include "gmp_types.hpp"
#include <unordered_map>
#include <string>

namespace Geez
{
    struct GeezMapData : IResource 
    {
    private:
        std::vector<wall_t>     walls;
        std::vector<sector_t>   sectors;
        std::unordered_map<U32, decal_t>     decals;
        std::unordered_map<U32, std::string> texture_references;
        std::unordered_map<U32, std::shared_ptr<sector_mesh_t>> sector_meshes; // Sector_id

    public:
        GeezMapData(const std::string& file);
        ~GeezMapData();

        void make_mesh();
        bool is_sector_scary(U32 sector_id, U32 wall_id);

        inline U32 get_wall_count() const { return walls.size(); }
        inline U32 get_sector_count() const { return sectors.size(); }
        inline U32 get_sector_mesh_count() const { return sector_meshes.size(); }

        wall_t* get_wall(U32 wall_id);
        const wall_t* get_wall(U32 wall_id) const;

        sector_t* get_sector(U32 sector_id);
        const sector_t* get_sector(U32 sector_id) const;

        sector_mesh_t* get_sector_mesh(U32 sector_id);
        const sector_mesh_t* get_sector_mesh(U32 sector_id) const;
       
        std::weak_ptr<sector_mesh_t> get_weak_sector_mesh(U32 sector_id);
        std::weak_ptr<const sector_mesh_t> get_weak_sector_mesh(U32 sector_id) const;
        
        inline const std::vector<wall_t> get_walls() const     { return walls;    }
        inline const std::vector<sector_t> get_sectors() const { return sectors;  }
    };
}

#endif