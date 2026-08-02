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

// Not a appconfig more like a build configuration? (not sure, subject to change)
#ifdef OVERRIDE_MAX_MIXER_CHANNEL
    #define MAX_MIXER_CHANNEL OVERRIDE_MAX_MIXER_CHANNEL
#else
    #define MAX_MIXER_CHANNEL 32
#endif

static const vec3 WORLD_UP(0.0f,1.0f,0.0f);
static const char* unlit_shader_name = "unlit_texture";
static const char* light_shader_name = "lit_texture";

static vec2 last_rel_mouse;
static F32 delta_time;  // Seconds
static F32 game_time;   // Seconds since start
static F32 frame_timer; // Timer every second
static I32 frame_tick;  // Tick count (resets every second)
static F32 runtime_fps;
static F32 cam_sensitivity = 0.03f;
static ResourceID current_map = "sample";
static GameObject* player = nullptr;

static bool isRunning = true;
static std::unique_ptr<Window> window;
static std::unique_ptr<AudioPlayer> audio;
static std::unique_ptr<ConfigLoader> config;
static std::unique_ptr<ResourceManager> resource;
static std::unique_ptr<GameObjectManager> entities;
static std::unique_ptr<MeshManager> meshes;
static std::unique_ptr<Renderer> renderer;
static Camera camera(WORLD_UP);
static Input input{};
static PhysicsSystem physics{};

// ================ TEMP ===================//

vec3 light_pos = vec3(0, 1, 0);
U32 sample_decal = 0;
std::string update_time = "";
std::string physics_update_time = "";
std::string render_time = "";

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
    const std::filesystem::path cfg_path = GetBasePath("geez.ini");
    config = std::make_unique<ConfigLoader>(cfg_path);
    config->set_defaults({
        {"controls", {
            {"sensitivity", "0.1"}}
        },
        {"physics", {
            {"gravity", "-9.8"}}
        },
        {"audio", {
            {"master_volume", "1.0"}}
        }
    });

    config->read_value("controls", "sensitivity", cam_sensitivity);
    config->read_value("physics", "gravity", physics.gravity);

    // OpenGL Configuration
    GL(glEnable(GL_DEPTH_TEST));
    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));
    GZ_Audio_Init();

    // Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    F32 main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDL3_InitForOpenGL(window->handle(), window->glContext());
    ImGui_ImplOpenGL3_Init("#version 330");

    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

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
            Internal::DECAL
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
    resource->set_watch_interval(500);
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    renderer.reset();
    entities.reset();
    resource.reset();
    meshes.reset();
    window.reset();
    audio.reset();
    config.reset();

    GZ_Audio_Quit();
    SDL_Quit();
    GZ_LOG_FORCE(GZ_SUCCESS, "Application terminated...");
}

void start()
{
    // Decal
    GeezMapData* map_data = resource->get<GeezMapData>(current_map);
    
    // Center of sector ID 0
    vec2 c = map_data->get_sector(0)->center;

    // Player   
    player = entities->create("Player", unlit_shader_name, "DefaultTexture");
    entities->attach_physics_component(player->id());
    player->position    = vec3(c.x,  0.8f, c.y); 
    player->scale       = vec3(0.1f, 0.1f, 0.1f);
    player->visible     = false;
    player->physics->height             = 0.3f;
    player->physics->collision_radius   = 0.05f;
    player->physics->step_height        = 0.125f;
    player->physics->friction           = 0.0f;

    // Sun
    auto instance1 = entities->create("LightSource", unlit_shader_name, "sun");
    instance1->scale = vec3(0.2f, 0.2f, 1.0f);

    // Random Shu image
    auto shu = entities->create("Shu_Arknights", unlit_shader_name, "ShuAK");
    shu->scale = vec3(0.5f, 0.5f, 1.0f);
    shu->position = vec3(c.x, 0.25f, c.y);
}

void logic_map_change() {
    bool changed = false;
    if ((changed = input.check_key(SDLK_1, GZ_TAP))) {
        current_map = "sample";
    }
    else if ((changed = input.check_key(SDLK_2, GZ_TAP))) {
        current_map = "centerHole";
    }
    else if ((changed = input.check_key(SDLK_3, GZ_TAP))) {
        current_map = "stairs";
    }
    else if ((changed = input.check_key(SDLK_4, GZ_TAP))) {
        current_map = "hole_in_hole";
    }
    else if ((changed = input.check_key(SDLK_5, GZ_TAP))) {
        current_map = "map_editor";
    }

    if (changed) {
        vec2 c = resource->get<GeezMapData>(current_map)->get_sector(0)->center;
        player->position = vec3(c.x, 0.1f, c.y);
    }
}

void logic_character_controller(GeezMapData* map) {
    // Camera movement
    F32 moveSpd = 1.5f;
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

    // Player is guranteed to have physics component anyways,
    // so no need to check for physics_component_t presence
    player->physics->velocity.x = (moveDir.x * moveSpd);
    player->physics->velocity.z = (moveDir.z * moveSpd);
}

void update()
{   
    // Change Map
    logic_map_change();
    GeezMapData* map_data = resource->get<GeezMapData>(current_map);

    // Toggle Mouse cursor visibility
    if (input.check_key(SDLK_ESCAPE, GZ_TAP)) {
        window->toggle_cursor_visible();
        GZ_LOG(GZ_DEBUG, "Cursor Visibility Toggled");
    }

    logic_character_controller(map_data);

    // Spawn physics balls in front of camera
    static int balls = 0;
    if (
        input.check_mouse_left(GZ_HOLD) && 
        !window->is_cursor_shown() &&
        !input.check_key(SDLK_LSHIFT, GZ_HOLD)
    ) {
        // GameObject* ball = nullptr;
        // if ((ball = entities->get("Ball"))) {
        //     entities->destroy("Ball");
        // }

        GameObject* ball = entities->create("Ball" + std::to_string(balls++));
        if (ball){
            entities->attach_physics_component(ball->id());
            ball->position = camera.position + (camera.axis_forward() * 0.1f);
            ball->scale = vec3(0.1f, 0.1f, 1.0f);
            ball->physics->height       = 0.1f;
            ball->physics->mass         = 1.0f;
            physics.add_impulse_force(*entities, ball->id(), camera.axis_forward() * 4.0f);
        }
    }

    {   // ============= TEMP ============= //
        // Example of moving a sector by floor height
        // Left mouse means a positive addition
        // Moving a floor will not move it's height
        if (map_data->get_sector(1) != nullptr && input.check_key(SDLK_LSHIFT, GZ_HOLD)) {       
            F32 add = 0.5f;
            
            map_data->set_sector_floor(1, ((
                (input.check_mouse_left(GZ_HOLD) - input.check_mouse_right(GZ_HOLD)) * add
            ) * delta_time), true);

            map_data->set_sector_ceil(1, ((
                (input.check_mouse_left(GZ_HOLD) - input.check_mouse_right(GZ_HOLD)) * add
            ) * delta_time), true);
        }

        // Raycasting (Super slow !!!)cls
        if (input.check_key(SDLK_R, GZ_HOLD)) {
            const wall_t* hit_wall = nullptr;
            vec3 hitpoint;
            
            wall_raycast(camera.position, camera.axis_forward(), *map_data, &hitpoint, &hit_wall); 
            decal_t* decal = map_data->get_decal(sample_decal);

            // Firt time creating
            if (!decal && hit_wall) {
                sample_decal = map_data->make_decal(
                    hitpoint, 
                    vec2(1, 1), 
                    "blood_splat", 
                    decal_t::WALL, 
                    hit_wall->id
                );
                decal = map_data->get_decal(sample_decal);
            }

            // Update
            if (decal && hit_wall) {
                map_data->update_decal(decal, hitpoint, decal_t::WALL, hit_wall->id);
            }
        }

        if (input.check_key(SDLK_SPACE, GZ_TAP)) {
            audio->play(resource->get<Audio>("syfm"));
        }
    }
}

void post_update() {
    GeezMapData* map_data = resource->get<GeezMapData>(current_map);
    map_data->update_sectors();

    camera.position = player->position + vec3(0, (player->physics->height * 0.5f), 0);
    I32 stencil_size;
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencil_size);
    I32 stencilBits = 0;
    GL(glGetFramebufferAttachmentParameteriv(
        GL_FRAMEBUFFER, 
        GL_STENCIL, 
        GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, 
        &stencilBits
    ));

    ImGui::Begin("Testing");
    ImGui::SeparatorText("exposed engine"); ImGui::NewLine();
    ImGui::Text("FPS:          %.4f",  runtime_fps);
    ImGui::Text("Ticks:        %d",    frame_tick);
    ImGui::Text("Game Time:    %.4fs", game_time);
    ImGui::Text("Delta Time:   %.4fs", delta_time);
    ImGui::NewLine(); ImGui::SeparatorText("internal engine"); ImGui::NewLine();
    ImGui::Text("Mixer Channels:       %d", MAX_MIXER_CHANNEL);
    ImGui::Text("Active Mixer Channel: %d", AudioPlayer::Internal::get_active_mixer_channel_count(*audio.get()));
    ImGui::Text("SDL Stencil:  %d", stencil_size);
    ImGui::Text("OGL Stencil:  %d", stencilBits);
    ImGui::Text("Update Time:        %s", update_time.c_str());
    ImGui::Text("PhysicsUpdate Time: %s", physics_update_time.c_str());
    ImGui::Text("Render Time:        %s", render_time.c_str());
    ImGui::Text("GameObject count:           %lld", entities->count());
    ImGui::Text("GameObject (Physics) count: %lld", entities->physics_count());
    ImGui::NewLine(); ImGui::SeparatorText("config"); ImGui::NewLine();
    ImGui::Text("Sensitivity %.4f", cam_sensitivity);
    ImGui::End();
}

void render()
{
    // Light Shader Uniforms
    Shader* light_shader = resource->get<Shader>(light_shader_name);
    if (light_shader) {
        light_shader->bind();
        light_shader->
         set_uniform<F32>("u_normalFlip", 1.0)
        .set_uniform<F32>("u_time", game_time)
        .set_uniform<vec3>("u_view_pos", camera.position)
        .set_uniform<vec3>("u_light.position", light_pos)
        .set_uniform<vec3>("u_light.ambient",  vec3(0.3f))
        .set_uniform<vec3>("u_light.diffuse",  vec3(1.0f, 1.0f, 0.75f))
        .set_uniform<vec3>("u_light.specular", vec3(0.1f));
    }
    
    // Gameobjects
    renderer->submit_map_geometry(*resource->get<GeezMapData>(current_map));
    for (const GameObject* object : *entities) {
        if (!object->visible) continue;

        renderer->submit(std::make_unique<RenderDataGameobject_t>([&]{
            RenderDataGameobject_t d{};
            d.shader_id      = object->shader_id;
            d.texture_ids[0] = object->texture_id;
            d.position       = object->position;
            d.scale          = object->scale;
            return d;
        }()));
    }

    // Crosshair
    renderer->submit(std::make_unique<RenderDataGUI_t>([&]{
        RenderDataGUI_t d{};
        d.shader_id      = unlit_shader_name;
        d.texture_ids[0] = "Crosshair";
        d.screen_pos     = vec2(0, 0);
        d.angle          = 0;
        d.size           = vec2(25.0f);
        return d;
    }()));

    // Done
    renderer->flush();
}

// ------------------------------------------------------------------------------//
//                            M A I N                                            //
// ------------------------------------------------------------------------------//

int main() 
{
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        camera.update();
        {
            RaiiTimer<SECONDS> rt(&update_time);
            update();
        }
        {
            RaiiTimer<SECONDS> rt(&physics_update_time);
            physics.update(*entities, *resource->get<GeezMapData>(current_map), delta_time);
        }
        post_update();
        {
            RaiiTimer<SECONDS> rt(&render_time);
            render();
            renderer->display_frame();
        }
        
        // runtime FPS Calculation
        frame_tick++;
        frame_timer += delta_time;
        if (frame_timer >= 1.0) {
            runtime_fps = frame_tick / frame_timer;
            frame_timer = 0.0;
            frame_tick = 0;
        }
    }

    onQuit();
    return 0;
    
}