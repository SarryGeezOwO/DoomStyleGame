#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <cstdint>
#include <array>

namespace Geez
{
    using F32   = float;
    using F64   = double;

    using I8    = int8_t;
    using I16   = int16_t;
    using I32   = int32_t;
    using I64   = signed long long;

    using U8    = unsigned char;
    using U16   = unsigned short;
    using U32   = unsigned int;
    using U64   = unsigned long long;

    using UPTR  = uintptr_t;

    using Color1f = float;
    using Color3f = glm::vec3;
    using Color4f = glm::vec4;
}