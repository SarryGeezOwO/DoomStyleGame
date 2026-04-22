#include "geometry_util.hpp"
#include "common_types.hpp"
#include "utility.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

using namespace glm;

namespace Geez
{
    static constexpr F32 EPSILON = std::numeric_limits<F32>::epsilon();
 
    vec2 get_polygon_center(const Polygon_t& polygon) 
    {
        if (polygon.empty()) {
            return {0.0, 0.0};
        }

        F32 area = 0.0;
        F32 centroidX = 0.0;
        F32 centroidY = 0.0;

        I32 n = static_cast<I32>(polygon.size());
        for (I32 i = 0; i < n; ++i) {
            Point_t p1 = polygon[i];
            Point_t p2 = polygon[(i + 1) % n];

            F32 crossProduct = p1[0] * p2[1] - p2[0] * p1[1];
            area += crossProduct;
            centroidX += (p1[0] + p2[0]) * crossProduct;
            centroidY += (p1[1] + p2[1]) * crossProduct;
        }

        area /= 2.0;

        if (area == 0.0) {
            return {0.0, 0.0};
        }

        centroidX /= (6.0 * area);
        centroidY /= (6.0 * area);
        return {centroidX, centroidY};
    }

    mat4 make_rotation_from_quaternion(const vec3& rotation) {
        vec3 rad = radians(rotation);
        quat euler = quat(rad);
             euler = normalize(euler);
        return mat4_cast(euler);
    }

    mat4 make_rotation_from_direction(vec3 dir) 
    {
        vec3 forward = normalize(dir);

        vec3 up = vec3(0,1,0);
        if (abs(dot(forward, up)) > 0.999f) {
            up = vec3(1, 0, 0); 
        }

        vec3 right = normalize(cross(up, forward));
        vec3 new_up = cross(forward, right);

        mat4 rot(1.0f);

        rot[0] = vec4(right,   0);
        rot[1] = vec4(new_up,  0);
        rot[2] = vec4(forward, 0);

        return rot;
    }

    bool check_point_inside_polygon(const Polygon_t& polygon, const vec2& p) 
    {
        F32 minX = polygon[0][0];
        F32 maxX = polygon[0][0];
        F32 minY = polygon[0][1];
        F32 maxY = polygon[0][1];
        for ( size_t i = 1; i < polygon.size(); i++ )
        {
            vec2 q(polygon[i][0], polygon[i][1]);
            minX = min( q.x, minX );
            maxX = max( q.x, maxX );
            minY = min( q.y, minY );
            maxY = max( q.y, maxY );
        }

        if ( p.x < minX || p.x > maxX || p.y < minY || p.y > maxY )
        {
            return false;
        }

        // https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html
        bool inside = false;
        for ( size_t i = 0, j = polygon.size() - 1 ; i < polygon.size() ; j = i++ )
        {
            if (polygon[i][0] == p.x && polygon[i][1] == p.y) {
                // One of the vertices are equal to the position
                return false;
            }

            if ( ( polygon[ i ][1] > p.y ) != ( polygon[ j ][1] > p.y ) &&
                p.x < ( polygon[ j ][0] - polygon[ i ][0] ) * ( p.y - polygon[ i ][1] ) / ( polygon[ j ][1] - polygon[ i ][1] ) + polygon[ i ][0] )
            {
                inside = !inside;
            }
        }

        return inside;
    }
    
    glm::vec2 closest_point_on_segment(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &p)
    {
        vec2 ab = b - a;
        F32 t = glm::dot(p - a, ab) / dot(ab, ab);
        t = glm::clamp(t, 0.0f, 1.0f);
        return a + ab * t;
    }

    bool is_point_in_sector(const glm::vec2& posXZ, const sector_t& sector) {
        bool isInside   = check_point_inside_polygon(sector.polygons[0], posXZ);
        if (!isInside)  return false;
        
        // Skip sector if inside of it's holes
        for (size_t i = 1; i < sector.polygons.size(); i++) {
            if (check_point_inside_polygon(sector.polygons[i], posXZ)) {
                isInside = false;
                break;
            }
        }
        return isInside;
    }

    bool raycast_2D(
        const vec2 &origin, 
        const vec2 &dir, 
        const vec2 &pa, 
        const vec2 &pb, 
        vec2* out_hit)
    {
        vec2 seg = pb - pa;
        F32 denom = cross(dir, seg);

        // Parallel
        if (fabs(denom) < EPSILON) return false;

        vec2 diff = pa - origin;
        F32 t = cross(diff, seg) / denom;
        F32 u = cross(diff, dir) / denom;

        bool hit = (t > EPSILON) && (u >= 0.0f) && (u < 1.0f);
        if (out_hit) {
            *out_hit = ((F32)hit * (origin + dir * t));
        }
        return hit;
    }

    bool wall_raycast(
        const vec3 &ray_origin,
        const vec3 &ray_dir,
        const GeezMapData &map,
        vec3* out_hit,
        const wall_t** out_wall)
    {
        vec2 ro(ray_origin.x, ray_origin.z);
        vec2 rd(ray_dir.x, ray_dir.z);

        F32 smallest_dist = FLT_MAX;
        bool hit = false;

        for (const wall_t& w : map.get_walls())
        {
            vec2 pa = point_to_vec(w.point_a);
            vec2 pb = point_to_vec(w.point_b);

            vec2 line_dir = pb - pa;
            vec2 diff     = pa - ro;

            F32 denom = cross(rd, line_dir);

            if (fabs(denom) < EPSILON)
                continue;

            F32 t = cross(diff, line_dir) / denom;
            F32 u = cross(diff, rd) / denom;

            if (t < 0)
                continue;

            if (u < 0 || u > 1)
                continue;

            vec2 hitpoint = ro + rd * t;
            F32 hitY = ray_origin.y + ray_dir.y * t;

            F32 floorY = 0;
            F32 ceilY  = 0;

            if (w.is_portal)
            {
                auto sa = map.get_sector(w.connected_sectors[0]);
                auto sb = map.get_sector(w.connected_sectors[1]);

                if (sa && sb)
                {
                    floorY = max(sa->floor_height, sb->floor_height);
                    ceilY  = min(sa->ceil_height, sb->ceil_height);

                    if (number_in_range(hitY, floorY, ceilY))
                        continue;
                }
            }

            vec3 closest_hit(hitpoint.x, hitY, hitpoint.y);
            vec3 v = closest_hit - ray_origin; 
            F32  dist_sq = dot(v, v);

            if (dist_sq < smallest_dist)
            {
                smallest_dist = dist_sq;
                if (out_hit)  *out_hit  = closest_hit;
                if (out_wall) *out_wall = &w;
                hit = true;
            }
        }

        return hit;
    }

    vec2 get_facing_normal(vec2 a, vec2 b, vec2 ref, bool is_flipped)
    {
        vec2 dir    = normalize(b - a);
        vec2 left   = vec2(-dir.y, dir.x);
        vec2 right  = vec2(dir.y, -dir.x);

        vec2 center = (a + b) * 0.5f;
        vec2 to_ref = normalize(ref - center);

        if (dot(left, to_ref) < 0)
            return is_flipped ? left : right;;

        return is_flipped ? right : left;
    }

}