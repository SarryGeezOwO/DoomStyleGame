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
#include <glm/glm.hpp>
#include <earcut/earcut.hpp>

using namespace glm;

enum GroupType {
    NONE    = -1,
    TEXTURE_REF,
    POINT,
    WALL,
    SECTOR,
    HOLE,
};

static std::vector<Geez::F32> get_values_from_string(const std::string& str)
{
    std::vector<Geez::F32> values;
    std::stringstream ss(str);
    Geez::F32 v;
    while (ss >> v) {
        values.push_back(v);
    }
    return values;
}

Geez::GeezMapData::GeezMapData(const std::string &file)
{
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
            // Expected 5 values
            // Wall_id, PointA, PointB, TextureID, IsPortal
            wall_t wall;
            wall.id         = id;
            wall.point_a    = points[static_cast<U32>(values[1])];
            wall.point_b    = points[static_cast<U32>(values[2])];
            wall.texture_id = texture_references[static_cast<U32>(values[3])];
            wall.is_portal  = static_cast<bool>(values[4]);
            walls.push_back(std::move(wall));
        }
        else if (type == SECTOR) 
        {
            // Expect 7 + wall_count
            // Sector_ID, wall_count, Floor, Ceil, isHole, TextureID(floor), TextureID(ceil), {wall_ids}

            U32 wall_count = static_cast<U32>(values[1]);
            sector_t sector;
            sector.id               = id;
            sector.floor_height     = values[2];
            sector.ceil_height      = values[3];
            sector.is_hole          = static_cast<bool>(values[4]);
            sector.texture_id_floor = texture_references[static_cast<U32>(values[5])];
            sector.texture_id_ceil  = texture_references[static_cast<U32>(values[6])];

            for (U32 i = 0; i < wall_count; i++){
                U32 wall_id = static_cast<U32>(values[i+7]);
                wall_t* w = get_wall(wall_id);
                if (w->connected_sectors_count < 2) {
                    w->connected_sectors[w->connected_sectors_count++] = id;
                }
                sector.walls.push_back(wall_id);
            }
            sectors.push_back(sector);
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
}

Geez::GeezMapData::~GeezMapData()
{
    sector_meshes.clear();
    walls.clear();
    sectors.clear();
    GZ_LOG(GZ_SUCCESS, "GeezMap [%s] Destroyed", m_resource_id.c_str());
}

void Geez::GeezMapData::make_mesh()
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
}

bool Geez::GeezMapData::is_sector_scary(U32 sector_id, U32 wall_id)
{
    wall_t* wall = get_wall(wall_id); 
    return (std::find(wall->scary_sectors.begin(), wall->scary_sectors.end(), sector_id) == wall->scary_sectors.end());
}

Geez::wall_t *Geez::GeezMapData::get_wall(U32 wall_id)
{
    for (wall_t& wall : walls) {
        if (wall.id == wall_id) {
            return &wall;
        }
    }
    GZ_LOG(GZ_FAIL, "Unknown Wall [ID: %d]", wall_id);
    return nullptr;
}

const Geez::wall_t *Geez::GeezMapData::get_wall(U32 wall_id) const
{
    for (const wall_t& wall : walls) {
        if (wall.id == wall_id) {
            return &wall;
        }
    }
    GZ_LOG(GZ_FAIL, "Unknown Wall [ID: %d]", wall_id);
    return nullptr;
}

Geez::sector_t *Geez::GeezMapData::get_sector(U32 sector_id)
{
    for (sector_t& sector : sectors) {
        if (sector.id == sector_id) {
            return &sector;
        }
    }
    GZ_LOG(GZ_FAIL, "Unknown Sector [ID: %d]", sector_id);
    return nullptr;
}

const Geez::sector_t *Geez::GeezMapData::get_sector(U32 sector_id) const
{
    for (const sector_t& sector : sectors) {
        if (sector.id == sector_id) {
            return &sector;
        }
    }
    GZ_LOG(GZ_FAIL, "Unknown Sector [ID: %d]", sector_id);
    return nullptr;
}

Geez::sector_mesh_t *Geez::GeezMapData::get_sector_mesh(U32 sector_id)
{
    if (sector_meshes.find(sector_id) != sector_meshes.end()) {
        return sector_meshes.at(sector_id).get();
    }
    GZ_LOG(GZ_FAIL, "Unknown Sector_Mesh [ID: %d]", sector_id);
    return nullptr;
}

const Geez::sector_mesh_t *Geez::GeezMapData::get_sector_mesh(U32 sector_id) const
{
    if (sector_meshes.find(sector_id) != sector_meshes.end()) {
        return sector_meshes.at(sector_id).get();
    }
    GZ_LOG(GZ_FAIL, "Unknown Sector_Mesh [ID: %d]", sector_id);
    return nullptr;
}

std::weak_ptr<Geez::sector_mesh_t> Geez::GeezMapData::get_weak_sector_mesh(U32 sector_id)
{
    if (sector_meshes.find(sector_id) != sector_meshes.end()) {
        return sector_meshes.at(sector_id);
    }
    GZ_LOG(GZ_FAIL, "Unknown Sector_Mesh [ID: %d]", sector_id);
    return std::weak_ptr<sector_mesh_t>();
}

std::weak_ptr<const Geez::sector_mesh_t> Geez::GeezMapData::get_weak_sector_mesh(U32 sector_id) const
{
    if (sector_meshes.find(sector_id) != sector_meshes.end()) {
        return sector_meshes.at(sector_id);
    }
    GZ_LOG(GZ_FAIL, "Unknown Sector_Mesh [ID: %d]", sector_id);
    return std::weak_ptr<sector_mesh_t>();
}
