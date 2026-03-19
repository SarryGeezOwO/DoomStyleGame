#version 330 core
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec3 a_normal;
out vec2 v_texCoord;
out vec3 v_normal;
out vec3 v_fragPos;

uniform float u_normalFlip; // 1.0 = normal, -1.0 = flipped
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
    vec4 world_pos = u_model * vec4(a_position, 1.0);
    v_fragPos = world_pos.xyz;

    // Transform the normal properly
    mat3 normalMatrix = mat3(transpose(inverse(u_model)));
    v_normal = normalize(normalMatrix * (a_normal * u_normalFlip));

    v_texCoord = a_texCoord;
    gl_Position = u_proj * u_view * world_pos;
}