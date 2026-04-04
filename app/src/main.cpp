#include "geez.hpp"

/*
    haha, very inconsistent design choices
    I want to die 💔💔🌹

    Project be damned, zero version control or whatever
    Let's just see how long this shitshow last 💩💩💀

    Also this entire shit has zero unit testing or any testing
    in general, only logging. 🤞🤞 

    [2025-12-20]
    Yes, a doom engine having a FUCKING mesh management,
    like bro the only time I will be needing custom meshes
    would be map sectors, The rest are just fucking planes lmao.

    [2026-03-12]
    Abstraction here, abstraction there. And now I have to deal with
    painter's algorithm because transparency is a very cool thing in
    OpenGL👍👍

*/

using namespace Geez;
using namespace glm;

#define MAX_MIXER_CHANNEL 32
static const vec3 WORLD_UP(0.0f,1.0f,0.0f);
static const char* unlit_shader_name = "unlit_texture";
static const char* light_shader_name = "lit_texture";

static vec2 last_rel_mouse;
static F32 delta_time;  // Seconds
static F32 game_time;   // Seconds since start
static F32 frame_timer; // Timer every second
static I32 frame_tick;  // Tick count every second
static F32 runtime_fps;
static F32 cam_sensitivity = 0.075f;

static bool isRunning = true;
static std::unique_ptr<Window> window;
static std::unique_ptr<AudioPlayer> audio;
static std::unique_ptr<Renderer> renderer;
static std::unique_ptr<ResourceManager> resource;
static std::unique_ptr<GameObjectManager> entities;
static std::unique_ptr<MeshManager> meshes;
static Camera camera(WORLD_UP);
static Input input;

// ================ TEMP ===================//

std::string sample_map = "sample";
vec3 light_pos = vec3(0, 1, 0);

// =========================================//

void on_resize(const ivec2& newsize) {
    GL(glViewport(0, 0, newsize.x, newsize.y));
    GZ_LOG(GZ_DEBUG, "Window Resized!");
    camera.set_resolution(newsize);
}

void on_mouse_move(const vec2& screen, const vec2& relative) {
    if (!window->is_cursor_shown()) {
        camera.euler_angle.x += relative.x * cam_sensitivity;
        camera.euler_angle.y -= relative.y * cam_sensitivity;

        // Pitch Constraints
        if(camera.euler_angle.y > 89.0f)
            camera.euler_angle.y =  89.0f;
        if(camera.euler_angle.y < -89.0f)
            camera.euler_angle.y = -89.0f;
    }
}

void init() {
    // Configuration
    GL(glEnable(GL_DEPTH_TEST));
    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));
    GZ_Audio_Init();

    // Sub Systems
    window->set_cursor_visible(false);

    renderer  = std::make_unique<Renderer>();
    audio     = std::make_unique<AudioPlayer>(MAX_MIXER_CHANNEL);
    entities  = std::make_unique<GameObjectManager>();
    resource  = std::make_unique<ResourceManager>();
    meshes    = std::make_unique<MeshManager>(
        std::vector<Internal::PrimitveMesh>{
            Internal::QUAD,
            Internal::LINE,
            Internal::AXIS
        }
    );

    RenderContext context;
    context.active_camera   = &camera;
    context.active_window   = window.get();
    context.meshes          = meshes.get();
    context.resources       = resource.get();
    renderer->set_context(std::make_unique<RenderContext>(context));
    
    input.AddMouseMoveCallback(on_mouse_move);
    input.AddWindowResizeCallback(on_resize);
    resource->load_all();
    resource->set_watch_interval(3000);
    resource->start_watching();

    camera.perspective();
    camera.position.y+=0.5f;
    camera.position.z+=1.5f;
    camera.set_clipping_plane(0.01f, 1000.0f);
    camera.set_resolution(window->get_size());
    camera.set_fov(85.0f);
}

void onQuit() 
{
    GZ_LOG(GZ_SUCCESS, "Application closing...");
    renderer.reset();
    entities.reset();
    resource.reset();
    meshes.reset();
    window.reset();
    audio.reset();

    GZ_Audio_Quit();
    SDL_Quit();
    GZ_LOG_FORCE(GZ_SUCCESS, "Application terminated...");
}

void start()
{   
    // Sun gameobject
    auto instance1 = entities->create("LightSource", unlit_shader_name, "sun");
    instance1->scale = vec3(0.2f, 0.2f, 1.0f);

    auto shu = entities->create("Shu_Arknights", unlit_shader_name, "ShuAK");
    shu->scale = vec3(0.5f, 0.5f, 1.0f);
    vec2 centerMap = resource->get<GeezMapData>(sample_map)->get_sector(0)->center;
    shu->position = vec3(centerMap.x, 0.25f, centerMap.y);

    vec2 c = resource->get<GeezMapData>(sample_map)->get_sector(0)->center;
    camera.position = vec3(c.x, 0.1f, c.y);
}

void logic_map_change() {
    if (input.check_key(SDLK_1, GZ_TAP)) {
        sample_map = "sample";
        vec2 c = resource->get<GeezMapData>(sample_map)->get_sector(0)->center;
        camera.position = vec3(c.x, 0.1f, c.y);
    }
    else if (input.check_key(SDLK_2, GZ_TAP)) {
        sample_map = "centerHole";
        vec2 c = resource->get<GeezMapData>(sample_map)->get_sector(0)->center;
        camera.position = vec3(c.x, 0.3f, c.y);
    }
    else if (input.check_key(SDLK_3, GZ_TAP)) {
        sample_map = "stairs";
        vec2 c = resource->get<GeezMapData>(sample_map)->get_sector(0)->center;
        camera.position = vec3(c.x, 0.1f, c.y);
    }
    else if (input.check_key(SDLK_4, GZ_TAP)) {
        sample_map = "hole_in_hole";
        vec2 c = resource->get<GeezMapData>(sample_map)->get_sector(0)->center;
        camera.position = vec3(c.x, 0.1f, c.y);
    }
    else if (input.check_key(SDLK_5, GZ_TAP)) {
        sample_map = "map_editor";
        vec2 c = resource->get<GeezMapData>(sample_map)->get_sector(0)->center;
        camera.position = vec3(c.x, 0.1f, c.y);
    }
}

void logic_character_controller(GeezMapData* map) {
    // Camera movement
    F32 moveSpd = 1.25f;
    glm::vec3 moveDir(0.0f);
    const vec3 camForwardZX = camera.axis_forward() * vec3(1, 0, 1);

    // raw input
    if (input.check_key(SDLK_W, GZ_HOLD)) {
        moveDir += camForwardZX;
    }
    if (input.check_key(SDLK_S, GZ_HOLD)) {
        moveDir -= camForwardZX;
    }
    if (input.check_key(SDLK_A, GZ_HOLD)) {
        moveDir += camera.axis_right();
    }
    if (input.check_key(SDLK_D, GZ_HOLD)) {
        moveDir -= camera.axis_right();
    }

    // Normalize diagonal force
    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
    }

    // camera Vertcal collision
    bool isGrounded = false;
    const F32 gravity = 1.25f;
    const F32 cameraHeight = 0.25f;
    const F32 gravForce = gravity * delta_time;
    const F32 cameraGroundY = (camera.position.y - cameraHeight);
    vec2 camPosXZ = vec2(camera.position.x, camera.position.z);
    F32 bestFloor = -1000000.0f;
    bool foundSector = false;

    {   // =============== Vertical collision SCOPE =============== //
        for (const sector_t& sector : map->get_sectors()) {
            bool isInside  = is_point_in_sector(camPosXZ, sector);
            if (!isInside) continue;

            // Get the highest floor sector on your standig point
            if (sector.floor_height > bestFloor) {
                bestFloor   = sector.floor_height;
                foundSector = true;
            }
        }

        // With smooth snapping
        if (foundSector) {
            if (cameraGroundY - gravForce <= bestFloor) {
                camera.position.y = (bestFloor + cameraHeight);
                // camera.position.y = Geez::lerp(camera.position.y, (bestFloor + cameraHeight), delta_time * 15);
                isGrounded = true;
            }
        }
    
        if (!isGrounded) {
            camera.position.y -= gravForce;
        }
    }

    // camera update position
    camera.position += (moveDir * moveSpd * delta_time);

    // ================ Horizontal Collision ================ //
    const F32 camRadius = 0.05f;
    const F32 heightStepThreshold = 0.125f;
    camPosXZ.x = camera.position.x;
    camPosXZ.y = camera.position.z;

    for (const sector_t& sector : map->get_sectors()) {
        bool isCameraInSector = is_point_in_sector(camPosXZ, sector);
        if (!isCameraInSector) continue; 

        for (U32 wid : sector.walls) {
            wall_t& wall = *map->get_wall(wid);

            // Since walls absolutely only contains two sectors as "connected"
            // compare their heights, and only allow pass through when below threshold
            // pass if max floor is less than realCamY
            if (wall.is_portal) {
                const sector_t *sa = map->get_sector(wall.connected_sectors[0]);
                const sector_t *sb = map->get_sector(wall.connected_sectors[1]);

                if (sa && sb) {
                    const F32 sfDiff = abs(sa->floor_height - sb->floor_height);
                    if (max(sa->floor_height, sb->floor_height) < cameraGroundY + 0.005f) 
                        continue; // Steppng down
                    else if (sfDiff <= heightStepThreshold)
                        continue; // stepping up scenario
                }
            }

            const vec2 p1(wall.point_a[0], wall.point_a[1]);
            const vec2 p2(wall.point_b[0], wall.point_b[1]);

            vec2 closest = closest_point_on_segment(p1, p2, camPosXZ);
            vec2 diff    = camPosXZ - closest;
            F32 dist     = length(diff);

            if (dist < camRadius) {
                vec2 pushDir = normalize(diff);
                F32 penetration = camRadius - dist;
                camera.position.x += pushDir.x * penetration;
                camera.position.z += pushDir.y * penetration;
            }
        }   
    }
}

void update()
{   
    // Change Map
    logic_map_change();
    GeezMapData* map_data = resource->get<GeezMapData>(sample_map);

    // Toggle Mouse cursor visibility
    if (input.check_key(SDLK_ESCAPE, GZ_TAP)) {
        window->toggle_cursor_visible();
        GZ_LOG(GZ_DEBUG, "Cursor Visibility Toggled");
    }

    logic_character_controller(map_data);

    // Light movement (arrow keys)
    const F32 lightSpd = 1.25f * delta_time;
    if (input.check_key(SDLK_UP, GZ_HOLD))    light_pos.z += lightSpd;  // forward
    if (input.check_key(SDLK_DOWN, GZ_HOLD))  light_pos.z -= lightSpd;  // backward
    if (input.check_key(SDLK_LEFT, GZ_HOLD))  light_pos.x -= lightSpd;  // left
    if (input.check_key(SDLK_RIGHT, GZ_HOLD)) light_pos.x += lightSpd;  // right

    // Optional vertical control
    if (input.check_key(SDLK_K, GZ_HOLD)) light_pos.y += lightSpd;
    if (input.check_key(SDLK_L, GZ_HOLD)) light_pos.y -= lightSpd;

    // Move sun gameobject to actual lightPosition
    entities->get("LightSource")->position = light_pos;

    {   // ============= TEMP ============= //
        // Example of moving a sector by floor height
        // Left mouse means a positive addition
        // Moving a floor will not move it's height
        if (map_data->get_sector(1) != nullptr) {       
            F32 add = 0.5f;
            if (input.check_key(SDLK_LSHIFT, GZ_HOLD)) {
                map_data->get_sector(1)->ceil_height += (
                    (input.check_mouse_left(GZ_HOLD) - input.check_mouse_right(GZ_HOLD)) * add
                ) * delta_time;
            }
            else {
                map_data->get_sector(1)->floor_height += (
                    (input.check_mouse_left(GZ_HOLD) - input.check_mouse_right(GZ_HOLD)) * add
                ) * delta_time;
            }
        }

        if (input.check_key(SDLK_SPACE, GZ_TAP)) {
            audio->play(resource->get<Audio>("syfm"));
        }
    }
}

void render()
{
    // Light Shader Uniforms
    Shader* light_shader = resource->get<Shader>(light_shader_name);
    if (light_shader) {
        light_shader->bind();
        light_shader->
         set_uniform<F32>("u_normalFlip", 1.0)
        .set_uniform<vec3>("u_view_pos", camera.position)
        .set_uniform<vec3>("u_light.position", light_pos)
        .set_uniform<vec3>("u_light.ambient",  vec3(0.3f))
        .set_uniform<vec3>("u_light.diffuse",  vec3(1.0f, 1.0f, 0.75f))
        .set_uniform<vec3>("u_light.specular", vec3(0.1f));
    }
    
    // Gameobjects
    renderer->submit_map_geometry(*resource->get<GeezMapData>(sample_map));
    for (const GameObject* object : *entities) {
        RenderDataGameobject_t data;
        data.shader_id      = object->shader_id;
        data.texture_ids[0] = object->texture_id;
        data.position       = object->position;
        data.rotation       = object->rotation;
        data.scale          = object->scale;
        renderer->submit(std::make_unique<RenderDataGameobject_t>(data));
    }

    // Crosshair
    RenderDataGUI_t crosshair;
    crosshair.shader_id      = unlit_shader_name;
    crosshair.texture_ids[0] = "Crosshair";
    crosshair.screen_pos     = vec2(0, 0);
    crosshair.angle          = 0;
    crosshair.size           = vec2(25.0f);
    renderer->submit(std::make_unique<RenderDataGUI_t>(crosshair));
    
    // Done
    renderer->flush();
}

// ------------------------------------------------------------------------------//
//                            M A I N                                            //
// ------------------------------------------------------------------------------//

int main() 
{
    if (!SDL_Init(SDL_INIT_EVENTS)) {
        GZ_LOG(GZ_FATAL, "SDL3 Initialization\n%s", SDL_GetError());
        return -1;
    }

    // vec2(1600, 900);
    window = std::make_unique<Window>("3D Renderer", vec2(1080, 720));
    if (window->error()) return -1;

    init();
    start();
    F32 last_time = 0.0f; // In Seconds
    while(isRunning) {
        // Game Time and Delta Time
        last_time = game_time;
        game_time  = SDL_GetTicks() / 1000.0f;
        delta_time = game_time - last_time;
        resource->process_pending_reload();

        last_rel_mouse = input.mouse_relative();
        isRunning = input.poll_events();

        camera.update();
        update();
        render();
        
        // runtime FPS Calculation
        frame_tick++;
        frame_timer += delta_time;
        if (frame_timer >= 1.0) {
            runtime_fps = frame_tick / frame_timer;
            frame_timer = 0.0;
            frame_tick = 0;
            window->rename("3D Renderer | FPS: " + std::to_string(runtime_fps));
        }
    }

    onQuit();
    return 0;
    
}