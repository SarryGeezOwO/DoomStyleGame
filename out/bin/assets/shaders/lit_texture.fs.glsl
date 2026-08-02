#version 330 core
precision highp float;

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 o_color;

in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_fragPos;

uniform float u_time;
uniform bool u_use_world_uv;
uniform vec2 u_uv_scale;
uniform vec3 u_view_pos;
uniform sampler2D u_texture;
uniform Light u_light;

vec2 resolution = vec2(1080 / 4, 720 / 3);

const float bayerMatrix8x8[64] = float[64](
    0.0/ 64.0, 48.0/ 64.0, 12.0/ 64.0, 60.0/ 64.0,  3.0/ 64.0, 51.0/ 64.0, 15.0/ 64.0, 63.0/ 64.0,
  32.0/ 64.0, 16.0/ 64.0, 44.0/ 64.0, 28.0/ 64.0, 35.0/ 64.0, 19.0/ 64.0, 47.0/ 64.0, 31.0/ 64.0,
    8.0/ 64.0, 56.0/ 64.0,  4.0/ 64.0, 52.0/ 64.0, 11.0/ 64.0, 59.0/ 64.0,  7.0/ 64.0, 55.0/ 64.0,
  40.0/ 64.0, 24.0/ 64.0, 36.0/ 64.0, 20.0/ 64.0, 43.0/ 64.0, 27.0/ 64.0, 39.0/ 64.0, 23.0/ 64.0,
    2.0/ 64.0, 50.0/ 64.0, 14.0/ 64.0, 62.0/ 64.0,  1.0/ 64.0, 49.0/ 64.0, 13.0/ 64.0, 61.0/ 64.0,
  34.0/ 64.0, 18.0/ 64.0, 46.0/ 64.0, 30.0/ 64.0, 33.0/ 64.0, 17.0/ 64.0, 45.0/ 64.0, 29.0/ 64.0,
  10.0/ 64.0, 58.0/ 64.0,  6.0/ 64.0, 54.0/ 64.0,  9.0/ 64.0, 57.0/ 64.0,  5.0/ 64.0, 53.0/ 64.0,
  42.0/ 64.0, 26.0/ 64.0, 38.0/ 64.0, 22.0/ 64.0, 41.0/ 64.0, 25.0/ 64.0, 37.0/ 64.0, 21.0 / 64.0
);

vec3 dither(vec2 uv, vec3 color, int colorNum) {

    float ss = 0.25;
    float sh = 1;
    float offset = floor(u_time / ss) * sh;
    int x = int(uv.x * resolution.x) % 8;
    int y = int(uv.y * resolution.y + offset) % 8;
    float threshold = bayerMatrix8x8[y * 8 + x] - 0.88;

    color.rgb += threshold;
    color.r = floor(color.r * (colorNum - 1.0) + 0.5) / (colorNum - 1.0);
    color.g = floor(color.g * (colorNum - 1.0) + 0.5) / (colorNum - 1.0);
    color.b = floor(color.b * (colorNum - 1.0) + 0.5) / (colorNum - 1.0);
    return color;
}

void main() {
 
    vec2 uv;
    vec3 norm = normalize(v_normal);
    if (u_use_world_uv) {
        if (abs(norm.y) > 0.5)
            uv = v_fragPos.xz;
        else if (abs(norm.x) > abs(norm.z))
            uv = v_fragPos.zy;
        else
            uv = v_fragPos.xy;
    }
    else {
        uv = v_texCoord;
    }

    vec3 texColor = vec3(texture(u_texture, uv * u_uv_scale));

    // Ambient
    float alpha = texture2D(u_texture, uv * u_uv_scale).a;
    if (alpha <= 0.01f) {
        discard;
    }

    vec3 ambient = u_light.ambient * texColor;
    // vec3 ambient  = vec3(-(distance(v_fragPos, u_light.position) * 0.25)) + (ambient1 * 1.0);
    // Darkness simulation

    // Diffuse
    vec3 light_dir = normalize(u_light.position - v_fragPos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = u_light.diffuse * diff * texColor; 

    // Specular
    vec3 view_dir = normalize(u_view_pos - v_fragPos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = u_light.specular * spec;

    vec3 lighting = (ambient + diffuse + specular);
    o_color.rgb = dither(uv, texColor.rgb, 8) * 0.06;
    o_color += vec4(lighting, alpha);
}