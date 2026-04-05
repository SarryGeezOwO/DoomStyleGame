#pragma once

/*
@Author SarryGeezOwO
https://github.com/SarryGeezOwO

R.I.P project. Legit no version control 😔🙏🙏

IDK how inclusion or headers in general work
Don't judge me pls 💔💔💔
*/

// Core
#include "core/window.hpp"
#include "core/input.hpp"
#include "core/game_object.hpp"
#include "core/game_object_manager.hpp"
#include "core/mesh.hpp"
#include "core/mesh_manager.hpp"

// Physics
#include "physics/physics_comp.hpp"
#include "physics/physics.hpp"

// Renderer
#include "renderer/camera.hpp"
#include "renderer/vertex_buffer.hpp"
#include "renderer/index_buffer.hpp"
#include "renderer/vertex_array.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "renderer/renderer.hpp"
#include "renderer/render_datatypes.hpp"

// GMP
#include "gmp/gmp_types.hpp"
#include "gmp/gmp.hpp"

// Resource
#include "resource/resource.hpp"
#include "resource/resource_manager.hpp"

// Utils
#include "util/log.hpp"
#include "util/common_types.hpp"
#include "util/error.hpp"
#include "util/utility.hpp"
#include "util/geometry_util.hpp"
#include "util/common_types.hpp"

// Vendor
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3_mixer/SDL_mixer.h>

// STD
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <algorithm>