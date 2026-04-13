#pragma once

#include <string>
#include <SDL3/SDL_filesystem.h>
#include <filesystem>
#include <algorithm>

namespace Geez
{
    inline std::string GetResourcePath() {
        return SDL_GetBasePath() + std::string("\\assets");
    }

    inline std::string GetShaderPath(const std::string& file_name) {
        return SDL_GetBasePath() + ("\\assets\\shaders\\" + file_name);
    }

    inline std::string GetTexturePath(const std::string& file_name) {
        return SDL_GetBasePath() + ("\\assets\\textures\\" + file_name);
    }

    inline std::string GetAudioPath(const std::string& file_name) {
        return SDL_GetBasePath() + ("\\assets\\audio\\" + file_name);
    }

    inline std::string GetMapDataPath(const std::string& file_name) {
        return SDL_GetBasePath() + ("\\assets\\maps\\" + file_name);
    }

    inline std::string to_lower(const std::string& str) {
        std::string copy = str;
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){
            return static_cast<char>(std::tolower(c));
        });
        return copy;
    }

    inline void trim(std::string& s) {
        // Remove leading whitespace
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));

        // Remove trailing whitespace
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    inline size_t find_str_nth_occurance(const std::string& src, const std::string& substr, int n) {
        size_t prev_pos = 0;
        while (n-- > 0) {
            size_t pos = src.find(substr, prev_pos + substr.length());
            if (pos == std::string::npos) {
                return std::string::npos; // No occurance of substr afterwards
            }
            prev_pos = pos;
        }
        return prev_pos;
    }

    inline std::string filename_no_ext(std::filesystem::path path) {
        path = path.filename();
        while (path.has_extension()) {
            path = path.stem();
        }
        return path.string();
    }

    
    template <typename T, typename U>
    T lerp(const T& a, const T& b, const U& t) {
        return a + t * (b - a);
    } 

    inline bool number_in_range(F32 x, F32 min, F32 max) {
        return (x == std::clamp(x, min, max));
    }
}