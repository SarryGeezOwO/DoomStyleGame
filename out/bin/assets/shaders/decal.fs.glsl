#version 330 core
precision highp float;

out vec4 o_color;

in vec2 v_texCoord;
uniform vec2 u_uv_scale;
uniform sampler2D u_texture;

void main() {
    vec4 texColor = texture2D(u_texture, v_texCoord * u_uv_scale);
    o_color = texColor;

    if (o_color.a <= 0.1) {
        discard;
    }
}