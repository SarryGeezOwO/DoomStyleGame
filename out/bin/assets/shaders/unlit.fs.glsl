#version 330 core
precision highp float;

out vec4 o_color;
uniform vec3 u_color;

void main() {
    o_color = vec4(u_color, 1);
}