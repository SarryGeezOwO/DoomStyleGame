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

uniform bool u_use_world_uv;
uniform vec2 u_uv_scale;
uniform vec3 u_view_pos;
uniform sampler2D u_texture;
uniform Light u_light;

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
    o_color = vec4(lighting, alpha);
}