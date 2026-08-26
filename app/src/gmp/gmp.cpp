#include "gmp.hpp"
#include "util/common_types.hpp"
#include "util/utility.hpp"
#include "util/log.hpp"
#include "util/geometry_util.hpp"
#include "renderer/vertex_array.hpp"
#include "renderer/vertex_buffer.hpp"
#include "renderer/index_buffer.hpp"

#include <fstream>
#include <sstream>
#include <earcut/earcut.hpp>

using namespace glm;

namespace Geez {
    TagResolver::TagResolver()
    {
    }

    TagResolver::~TagResolver()
    {
    }

    void TagResolver::resolve_tag(U32 tag_id)
    {
        // Resolves all tags that has connection to this specific tag_id
    }

    void TagResolver::resolve_all_tag()
    {
        for (TagConnection& conn : connections) {
            ITagClient* a = tag_map[conn.a];
            ITagClient* b = tag_map[conn.b];
            // if A and B triggers a callback on the same frame
            // one callback only gets triggered in response
            
            U8 from = a->tag_isModified + (b->tag_isModified * 2);
            
            if (a->tag_isModified) {
                conn.cb(reinterpret_cast<UPTR>(a), reinterpret_cast<UPTR>(b), from);
            }
            else {
                if (b->tag_isModified)
                    conn.cb(reinterpret_cast<UPTR>(a), reinterpret_cast<UPTR>(b), from);
            }
        }
    }




    enum GroupType {
        NONE    = -1,
        TEXTURE_REF,
        POINT,
        WALL,
        SECTOR,
        HOLE,
    };

    static U32 decal_id_ref = 0;
    static std::vector<F32> get_values_from_string(const std::string& str)
    {
        std::vector<F32> values;
        std::stringstream ss(str);
        F32 v;
        while (ss >> v) {
            values.push_back(v);
        }
        return values;
    }

    GeezMapData::GeezMapData(const std::string &file)
    {
        tag_resolver = std::make_unique<TagResolver>();
        static const std::unordered_map<std::string, GroupType> groupTypeMap = {
            { "--END--",        NONE        },
            { "[TEXTURE]",      TEXTURE_REF },
            { "[POINT]",        POINT       },
            { "[WALL]",         WALL        },
            { "[SECTOR]",       SECTOR      },
            { "[SECTOR_HOLE]",  HOLE        }
        };

        std::ifstream stream(file);
        if (!stream) {
            GZ_LOG(GZ_FAIL, "Failed to load GMP file [%s]", file);
            return;
        }

        GroupType type = NONE;
        std::unordered_map<U32, Point_t> points;  // ID  |  X, Y
        std::string line;

        while(std::getline(stream >> std::ws, line)) {
            trim(line);

            // Comments, Low budget comment checking
            if (line.substr(0, 2) == "//") continue;

            // Attempt to change Type
            if (groupTypeMap.find(line) != groupTypeMap.end()) {
                type = groupTypeMap.at(line);
                continue;
            }
            
            // Data embedding
            std::vector<F32> values = get_values_from_string(line);
            U32 id = static_cast<U32>(values[0]);

            if (type == NONE) continue;
            else if (type == TEXTURE_REF) 
            {
                // Internal_id, Texture_id (string)
                size_t start = find_str_nth_occurance(line, "\"", 1);
                size_t end   = find_str_nth_occurance(line, "\"", 2);
                std::string texture_id = line.substr(start+1, (end - start)-1);
                texture_references[id] = texture_id;
            }
            else if (type == POINT) 
            {
                // We only expect 3 values, if there's extras, simply ignore duh
                points[id] = {values[1], values[2]};
            }
            else if (type == WALL) 
            {
                // Expected 6 values
                // Wall_id, Tag_id, PointA, PointB, TextureID, IsPortal
                wall_t wall;
                wall.id         = id;
                wall.tag_id     = values[1];
                wall.point_a    = points[static_cast<U32>(values[2])];
                wall.point_b    = points[static_cast<U32>(values[3])];
                wall.texture_id = texture_references[static_cast<U32>(values[4])];
                wall.is_portal  = static_cast<bool>(values[5]);
                walls.push_back(std::move(wall));
                tag_resolver->addTagClient(wall);
            }
            else if (type == SECTOR) 
            {
                // Expect 8 + wall_count
                // Sector_ID, Tag_id, wall_count, Floor, Ceil, isHole, TextureID(floor), TextureID(ceil), {wall_ids}

                U32 wall_count = static_cast<U32>(values[2]);
                sector_t sector;
                sector.id               = id;
                sector.tag_id           = values[1];
                sector.floor_height     = values[3];
                sector.ceil_height      = values[4];
                sector.is_interior          = static_cast<bool>(values[5]);
                sector.texture_id_floor = texture_references[static_cast<U32>(values[6])];
                sector.texture_id_ceil  = texture_references[static_cast<U32>(values[7])];

                for (U32 i = 0; i < wall_count; i++){
                    U32 wall_id = static_cast<U32>(values[i+8]);
                    sector.walls.push_back(wall_id);
                }
                sectors.push_back(sector);
                tag_resolver->addTagClient(sector);
            }
            else if (type == HOLE) {
                // Expect 2 + wall_count
                // Sector_id, Wall_Count, [Wall_IDs]

                U32 wall_count = static_cast<U32>(values[1]);
                for (sector_t& sector : sectors) {
                    if (id != sector.id) continue;
                    Polygon_t polygon_hole;

                    // We only care about Unique Point
                    for (U32 i = 0; i < wall_count; i++){
                        U32 wall_id = static_cast<U32>(values[i+2]);
                        wall_t* wall = get_wall(wall_id);

                        // Skip missing instance wall
                        if (wall == nullptr) {
                            continue;
                        }

                        // Insert unique Vertex to polygon_hole
                        Point_t vertex = wall->point_a;
                        if (std::find(polygon_hole.begin(), polygon_hole.end(), vertex) != polygon_hole.end()) {
                            vertex = wall->point_b;
                        }

                        // Skip Vertex if already existed
                        if (std::find(polygon_hole.begin(), polygon_hole.end(), vertex) != polygon_hole.end()) {
                            continue;
                        }
                        polygon_hole.push_back(vertex);

                        // Also add this wall to the sector as scary!!
                        wall->scary_sectors.push_back(sector.id);
                        sector.walls.push_back(wall_id);
                    }

                    sector.polygons.push_back(polygon_hole);
                    break;
                }
            }
        }

        // Add wall sector connections
        std::unordered_map<U32, U32> portal_prev_sec;
        for (const sector_t& sector : sectors) { 
            for (const U32 wall_id : sector.walls) {
                wall_t *wall = get_wall(wall_id);

                auto it = portal_prev_sec.find(wall_id);
                if (it != portal_prev_sec.end()) {
                    // Two sectors acknowledges this wall existance, IDK what im cooking
                    wall->connected_sectors[1] = sector.id;
                }
                else {
                    // First sector to mention this edge
                    wall->connected_sectors[0] = sector.id;
                    portal_prev_sec.insert({wall_id, sector.id});
                }
            }
        }

        // Sort walls, sectors by ID from smallest to highest
        // For faster lookup
        std::sort(walls.begin(), walls.end(), [](const auto& a, const auto& b) {
            return a.id < b.id;
        });

        std::sort(sectors.begin(), sectors.end(), [](const auto& a, const auto& b) {
            return a.id < b.id;
        });
    }

    GeezMapData::~GeezMapData()
    {
        sector_meshes.clear();
        walls.clear();
        sectors.clear();
        decals.clear();
        tag_resolver.reset();
        GZ_LOG(GZ_SUCCESS, "GeezMap [%s] Destroyed", m_resource_id.c_str());
    }

    void GeezMapData::make_mesh()
    {
        // Loading Buffers from map data, Floor
        for (sector_t& sector : sectors) {
            std::vector<U32> indices;
            std::vector<F32> vertices;

            // Collect unique floor vertices
            for (size_t j = 0; j < sector.walls.size(); j++) {
                U32 wall_id = sector.walls[j];
                wall_t* wall = get_wall(wall_id);
                if (wall == nullptr) {
                    continue;
                }
                
                Point_t vertex = wall->point_a;
                if (std::find(sector.polygons[0].begin(), sector.polygons[0].end(), vertex) != sector.polygons[0].end()) {
                    // Point A exists, try B
                    // for mapmakers, not caring for winding
                    vertex = wall->point_b;
                }
            
                // add this vertex to the polygon, if sector is marked as scary!!
                if (std::find(wall->scary_sectors.begin(), wall->scary_sectors.end(), sector.id) == wall->scary_sectors.end()) {
                    sector.polygons[0].push_back(vertex);
                }

                vertices.push_back(vertex[0]); // X
                vertices.push_back(0);         // Y model matrix will move this
                vertices.push_back(vertex[1]); // Z
                vertices.push_back(vertex[0]); // U
                vertices.push_back(vertex[1]); // V
                vertices.push_back(0); // Nx
                vertices.push_back(1); // Ny
                vertices.push_back(0); // Nz
            }

            indices = mapbox::earcut<U32>(sector.polygons);
            sector.center = get_polygon_center(sector.polygons[0]);

            // Construct Buffer
            auto mesh  = std::make_shared<sector_mesh_t>();
            mesh->vbo  = std::make_unique<VertexBuffer>(vertices.size() * sizeof(F32), vertices.data());
            mesh->ebo  = std::make_unique<IndexBuffer>(indices.size(), indices.data());
            mesh->vao  = std::make_unique<VertexArray>();

            VertexBufferLayout mesh_layout;
            mesh_layout.push_attr<F32>(3, GL_FALSE);
            mesh_layout.push_attr<F32>(2, GL_FALSE);
            mesh_layout.push_attr<F32>(3, GL_FALSE);
            mesh->vao->bind_buffer(mesh->vbo.get(), mesh->ebo.get(), mesh_layout);
            mesh->vao->unbind();
            mesh->ebo->unbind();
            mesh->vbo->unbind();

            // Insert to map with sectorID as key
            sector_meshes.emplace(sector.id, std::move(mesh));
        }
        update_all_wall_normal();
    }

    bool GeezMapData::is_sector_scary(U32 sector_id, U32 wall_id)
    {
        wall_t* wall = get_wall(wall_id); 
        return (std::find(wall->scary_sectors.begin(), wall->scary_sectors.end(), sector_id) == wall->scary_sectors.end());
    }

    void GeezMapData::update_wall_normal(wall_t &wall)
    {
        // Compute wall normal
        Internal::Logger::disable_logging();
        vec2 pa = point_to_vec(wall.point_a);
        vec2 pb = point_to_vec(wall.point_b);
        
        sector_t* sa = get_sector(wall.connected_sectors[0]);
        sector_t* sb = get_sector(wall.connected_sectors[1]);
        Internal::Logger::enable_logging();
        
        // bool here is 'flip'
        std::vector<std::pair<vec2, bool>> ref_point{};
        if (sa && sb) {
            F32 max_floor = max(sa->floor_height, sb->floor_height);
            F32 max_ceil  = max(sa->ceil_height, sb->ceil_height);
            F32 min_ceil  = min(sa->ceil_height, sb->ceil_height);
            bool hole_present = sa->is_interior || sb->is_interior;
            
            const vec2& floor_look = (max_floor == sb->floor_height) ? sa->center : sb->center;
            const vec2& ceil_look  = (max_ceil == sb->ceil_height) ? sb->center : sa->center;
            const vec2& center_hole = (sb->is_interior) ? sb->center : sa->center;

            // Floor
            ref_point.push_back({
                (hole_present) ? center_hole : floor_look,
                hole_present * ((sb->is_interior) ? 
                    (max_floor == sb->floor_height) : 
                    (max_floor == sa->floor_height)
                )
            });

            // Ceil
            ref_point.push_back({
                (hole_present) ? center_hole : ceil_look,
                hole_present * ((sb->is_interior) ? 
                    (min_ceil == sb->ceil_height) : 
                    (min_ceil == sa->ceil_height)
                )
            });
        } 
        else if (sa) ref_point.push_back({sa->center, false});
        else if (sb) ref_point.push_back({sb->center, false});

        for (size_t i = 0; i < ref_point.size(); i++) {
            vec2 norm = get_facing_normal(pa, pb, ref_point[i].first, ref_point[i].second);
            wall.m_normal[i] = vec3(norm.x, 0, norm.y);
        }
    }

    void GeezMapData::update_all_wall_normal()
    {
        for (wall_t& wall : walls) {
            update_wall_normal(wall);
        }
    }

    void GeezMapData::update_sectors()
    {
        while (!m_update_queue.empty()) {
            U32 sid = m_update_queue.front();
            m_update_queue.pop();

            sector_t* s = get_sector(sid);
            if (!s) return;
            
            for (U32 wid : s->walls) {
                wall_t* w = get_wall(wid);
                if (w) update_wall_normal(*w);
            }
        }
    }

    void GeezMapData::set_sector_floor(sector_t &sector, F32 new_floor, bool additive)
    {
        sector.floor_height = (new_floor + (sector.floor_height * additive));
        m_update_queue.push(sector.id);
    }

    void GeezMapData::set_sector_floor(U32 id, F32 new_floor, bool additive)
    {
        sector_t* s = get_sector(id);
        if (s) set_sector_floor(*s, new_floor, additive);
    }

    void GeezMapData::set_sector_ceil(sector_t &sector, F32 new_ceil, bool additive)
    {
        sector.ceil_height = (new_ceil + (sector.ceil_height * additive));
        m_update_queue.push(sector.id);
    }

    void GeezMapData::set_sector_ceil(U32 id, F32 new_ceil, bool additive)
    {
        sector_t* s = get_sector(id);
        if (s) set_sector_ceil(*s, new_ceil, additive);
    }

    U32 GeezMapData::make_decal(vec3 pos, vec2 size, ResourceID texture_id, decal_t::target_t target, U32 target_id, U32 tag_id)
    {
        decal_t d{};
        d.tag_id        = tag_id;
        d.id            = decal_id_ref++;
        d.size          = size;
        d.texture_id    = texture_id;

        update_decal(&d, pos, target, target_id);
        decals.push_back(d);
        return d.id;
    }

    void GeezMapData::update_decal(decal_t *decal, glm::vec3 new_pos, decal_t::target_t target, U32 target_id)
    {
        if (!decal) return;

        decal->position  = new_pos;
        decal->target    = target;
        decal->target_id = target_id;
        decal->normal    = vec3(0);

        switch (decal->target)
        {
            case decal_t::WALL:
                {
                    // Compute wall normal
                    Internal::Logger::disable_logging();
                    const wall_t* wall = get_wall(decal->target_id);
                    if (!wall) {
                        Internal::Logger::enable_logging();
                        break;
                    }
                    
                    sector_t* sa = get_sector(wall->connected_sectors[0]);
                    sector_t* sb = get_sector(wall->connected_sectors[1]);
                    Internal::Logger::enable_logging();

                    if (sa && sb) {
                        // Determine if ceil or floor space
                        F32 max_floor = max(sa->floor_height, sb->floor_height);
                        F32 min_floor = min(sa->floor_height, sb->floor_height);
                        bool floor_region = number_in_range(new_pos.y, min_floor, max_floor);

                        decal->normal = ((floor_region) ? 
                            wall->normal_front() : wall->normal_back());
                        break;
                    }
                    decal->normal = wall->normal_front();
                }
                break;

            case decal_t::FLOOR:
                decal->normal = vec3(0, 1, 0);
                break;

            case decal_t::CEIL:
                decal->normal = vec3(0, -1, 0);
                break;
            
            default:
                break;
        }
    }

    void GeezMapData::update_decal(U32 decal_id, glm::vec3 new_pos, decal_t::target_t target, U32 target_id)
    {
        decal_t* decal = get_decal(decal_id);
        update_decal(decal, new_pos, target, target_id);
    }



    decal_t *GeezMapData::get_decal(U32 decal_id)
    {
        return const_cast<decal_t*>(
            static_cast<const GeezMapData*>(this)->get_decal(decal_id));
    }

    const decal_t *GeezMapData::get_decal(U32 decal_id) const
    {
        auto it = std::lower_bound(decals.begin(), decals.end(), decal_id, [](const decal_t& decal, U32 id){
            return decal.id < id;
        });

        if (it != decals.end() && it->id == decal_id)
            return &(*it);

        GZ_LOG(GZ_FAIL, "Unknown Decal [ID: %d]", decal_id);
        return nullptr;
    }



    std::vector<decal_t *> GeezMapData::get_wall_decals(U32 wall_id)
    {
        const GeezMapData* self = this;
        const std::vector<const decal_t *> carr = self->get_wall_decals(wall_id);

        std::vector<decal_t*> arr;
        for (const decal_t* d : carr) {
            arr.push_back(const_cast<decal_t*>(d));
        }
        return arr;
    }

    std::vector<const decal_t *> GeezMapData::get_wall_decals(U32 wall_id) const
    {
        std::vector<const decal_t *> arr;
        for (const decal_t& d : decals) {

            if (d.target != decal_t::WALL) 
                continue;
            
            if (d.target_id == wall_id)
                arr.push_back(&d); 
        }
        return arr;
    }



    std::vector<decal_t *> GeezMapData::get_sector_decals(U32 sector_id)
    {
        const GeezMapData* self = this;
        const std::vector<const decal_t *> carr = self->get_sector_decals(sector_id);

        std::vector<decal_t *> arr;
        for (const decal_t* d : carr) {
            arr.push_back(const_cast<decal_t*>(d));
        }
        return arr;
    }

    std::vector<const decal_t *> GeezMapData::get_sector_decals(U32 sector_id) const
    {
        std::vector<const decal_t *> arr;
        for (const decal_t& d : decals) {

            if (d.target != decal_t::CEIL && d.target != decal_t::FLOOR) 
                continue;

            if (d.target_id == sector_id)
                arr.push_back(&d); 
        }
        return arr;
    }



    wall_t *GeezMapData::get_wall(U32 wall_id)
    {
        return const_cast<wall_t*>(
            static_cast<const GeezMapData*>(this)->get_wall(wall_id));
    }

    const wall_t *GeezMapData::get_wall(U32 wall_id) const
    {
        auto it = std::lower_bound(walls.begin(), walls.end(), wall_id, [](const wall_t& wall, U32 id){
            return wall.id < id;
        });

        if (it != walls.end() && it->id == wall_id)
            return &(*it);

        GZ_LOG(GZ_FAIL, "Unknown Wall [ID: %d]", wall_id);
        return nullptr;
    }



    sector_t *GeezMapData::get_sector(U32 sector_id)
    {
        return const_cast<sector_t*>(
            static_cast<const GeezMapData*>(this)->get_sector(sector_id));
    }

    const sector_t *GeezMapData::get_sector(U32 sector_id) const
    {
        auto it = std::lower_bound(sectors.begin(), sectors.end(), sector_id, [](const sector_t& wall, U32 id){
            return wall.id < id;
        });

        if (it != sectors.end() && it->id == sector_id)
            return &(*it);

        GZ_LOG(GZ_FAIL, "Unknown Sector [ID: %d]", sector_id);
        return nullptr;
    }



    sector_mesh_t *GeezMapData::get_sector_mesh(U32 sector_id)
    {
        return const_cast<sector_mesh_t*>(
            static_cast<const GeezMapData*>(this)->get_sector_mesh(sector_id));
    }

    const sector_mesh_t *GeezMapData::get_sector_mesh(U32 sector_id) const
    {
        if (sector_meshes.find(sector_id) != sector_meshes.end()) {
            return sector_meshes.at(sector_id).get();
        }
        GZ_LOG(GZ_FAIL, "Unknown Sector_Mesh [ID: %d]", sector_id);
        return nullptr;
    }



    std::weak_ptr<sector_mesh_t> GeezMapData::get_weak_sector_mesh(U32 sector_id)
    {
        if (sector_meshes.find(sector_id) != sector_meshes.end()) {
            return sector_meshes.at(sector_id);
        }
        GZ_LOG(GZ_FAIL, "Unknown Sector_Mesh [ID: %d]", sector_id);
        return std::weak_ptr<sector_mesh_t>();
    }

    std::weak_ptr<const sector_mesh_t> GeezMapData::get_weak_sector_mesh(U32 sector_id) const
    {
        if (sector_meshes.find(sector_id) != sector_meshes.end()) {
            return sector_meshes.at(sector_id);
        }
        GZ_LOG(GZ_FAIL, "Unknown Sector_Mesh [ID: %d]", sector_id);
        return std::weak_ptr<sector_mesh_t>();
    }




    
    void GeezMapData::Event::interact_decal(ITagClient* ptr)
    {
        if (ptr) ptr->tag_isModified = true;
    }
}