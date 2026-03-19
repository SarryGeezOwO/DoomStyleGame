#ifndef GZ_GEOMETRY_UTIL_HPP
#define GZ_GEOMETRY_UTIL_HPP

#include "gmp/gmp_types.hpp"
#include <glm/glm.hpp>

namespace Geez
{
    /**
     * @brief Compute the centroid (geometric center) of a 2D polygon.
     *
     * Uses the area-weighted formula (shoelace method). If the polygon is empty
     * or has zero area (degenerate), the function returns (0,0).
     *
     * @param polygon Polygon_t: sequence of points where each Point_t is [x,y].
     * @return glm::vec2 Centroid coordinates as (x, y).
     */
    glm::vec2 get_polygon_center(const Polygon_t& polygon);

    /// @brief Constructs a rotation matrix from a quaternion represented as a 3D vector.
    /// @param rotation A glm::vec3 representing the quaternion components (x, y, z) (degrees).
    /// @return A glm::mat4 rotation matrix corresponding to the given quaternion.
    glm::mat4 make_rotation_from_quaternion(const glm::vec3& rotation);

    /**
     * @brief Create a rotation matrix that points its forward axis toward `dir`.
     *
     * The function normalizes `dir` and constructs an orthonormal basis
     * (right, up, forward). If `dir` is nearly parallel to the world up vector
     * (0,1,0), an alternate up vector is used to avoid numerical instability.
     *
     * @param dir glm::vec3: Desired forward direction (not required to be normalized).
     * @return glm::mat4 Rotation matrix (4x4) with right, up, forward axes in rows 0..2.
     */
    glm::mat4 make_rotation_from_direction(glm::vec3 dir);

    /**
     * @brief Test whether a 2D point lies strictly inside a polygon.
     *
     * Uses a bounding-box rejection followed by the ray-casting (pnpoly)
     * algorithm. If the point exactly matches a polygon vertex the function
     * returns false (vertices are treated as outside).
     *
     * @param polygon Polygon_t: sequence of points where each Point_t is [x,y].
     * @param p glm::vec2: Point to test.
     * @return bool True if the point is strictly inside the polygon.
     */
    bool check_point_inside_polygon(const Polygon_t& polygon, const glm::vec2& p);

    glm::vec2 closest_point_on_segment(const glm::vec2& a, const glm::vec2& b, const glm::vec2& p);
}

#endif