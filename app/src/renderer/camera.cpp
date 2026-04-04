#include "camera.hpp"

using namespace glm;

Geez::Camera::Camera(const vec3 &world_up)
    : m_world_up(world_up)
{
    position = vec3(0.0f);
    euler_angle.x = -90.0f;
    m_resolution = ivec2(1080, 720);

    m_forward     = vec3(0,0,-1);   
    m_right       = normalize(cross(m_world_up, m_forward));
    m_up          = cross(m_forward, m_right);
    m_view_matrix = lookAt(position, position + m_forward, m_world_up); 
    update_projection();
}

Geez::Camera::~Camera()
{}

void Geez::Camera::update_projection()
{
    if (m_isOrthographic) {
        glm::vec2 half = m_resolution / 2;
        m_proj_matrix = glm::ortho(
            -half.x, half.x, -half.y, half.y, -1.0f, 1.0f
        );
        return;
    }

    m_proj_matrix = glm::perspective(
        m_fov, get_aspect_ratio(), m_near, m_far
    );
}

void Geez::Camera::update()
{
    vec3 dir;
    dir.x = cos(radians(euler_angle.x)) * cos(radians(euler_angle.y));
    dir.y = sin(radians(euler_angle.y));
    dir.z = sin(radians(euler_angle.x)) * cos(radians(euler_angle.y));

    m_forward   = normalize(dir);
    m_right     = normalize(cross(m_world_up, m_forward));
    m_up        = cross(m_forward, m_right);

    m_view_matrix = lookAt(position, position + m_forward, m_world_up);
}

void Geez::Camera::move(const glm::vec3 &direction, F32 amount)
{
    position += direction * amount;
}

// Move forwad relative to there the camera is looking at
void Geez::Camera::move_rel_forward(F32 amount, bool ignoreY)
{
    position += (m_forward * vec3(1, !ignoreY, 1)) * amount;
}

// Move right relative to there the camera is looking at
void Geez::Camera::move_rel_right(F32 amount)
{
    position -= m_right * amount;
}

void Geez::Camera::move_up(F32 amount)
{
    position += m_up * amount;
}

void Geez::Camera::set_clipping_plane(F32 near, F32 far)
{
    m_near = (near < 0) ? m_near : near;
    m_far  = (far < 0) ? m_far : far;
    update_projection();
}

void Geez::Camera::set_resolution(const glm::ivec2 &resolution)
{
    m_resolution = resolution;
    update_projection();
}

void Geez::Camera::set_fov(F32 fov)
{
    m_fov = radians(fov);
    update_projection();
}
