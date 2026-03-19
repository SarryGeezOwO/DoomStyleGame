#ifndef GZ_CAMERA_HPP
#define GZ_CAMERA_HPP

#include "util/common_types.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Geez
{
    struct Camera
    {
    private:
        glm::vec3 m_world_up;
        glm::vec3 m_forward;
        glm::vec3 m_right;
        glm::vec3 m_up;

        glm::mat4 m_view_matrix;
        glm::mat4 m_proj_matrix;
    
        glm::ivec2 m_resolution;
        F32 m_near = 0.1f;
        F32 m_far  = 1000.0f;
        F32 m_fov  = glm::radians(45.0f);
        bool m_isOrthographic = false;

        void update_projection();

    public:
        glm::vec3 position;
        glm::vec3 euler_angle;

        Camera() = delete;
        Camera(const glm::vec3& world_up);
        ~Camera();
        
        void update();
        void move(const glm::vec3& direction, F32 amount);
        void move_rel_forward(F32 amount, bool ignoreY);
        void move_rel_right(F32 amount);
        void move_up(F32 amount);

        // Values are not modified if less than zero
        void set_clipping_plane(F32 near, F32 far);
        void set_resolution(const glm::ivec2& resolution);
        void set_fov(F32 fov);

        inline F32  get_aspect_ratio() const { return static_cast<F32>(m_resolution.x) / static_cast<F32>(m_resolution.y); }
        inline F32  get_near() const { return m_near; }
        inline F32  get_far() const { return m_far; }
        inline F32  get_fov() const { return glm::degrees(m_fov); }
        
        inline void orthographic() { m_isOrthographic = true;  update_projection();  }
        inline void perspective()  { m_isOrthographic = false; update_projection(); }
        inline bool is_orthographic() const { return m_isOrthographic; }

        inline glm::vec3 axis_forward() const { return m_forward; }
        inline glm::vec3 axis_right() const { return m_right; }
        inline glm::vec3 axis_up() const { return m_up; }
        
        inline const glm::mat4& view_matrix() noexcept { 
            return m_view_matrix; 
        }

        inline const glm::mat4& projection_matrix() noexcept {
            return m_proj_matrix;
        }
    };
}

#endif