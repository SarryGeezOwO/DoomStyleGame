#ifndef GZ_CONFIG_HPP
#define GZ_CONFIG_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "mINI/ini.h"
#include "util/log.hpp"
#include "util/common_types.hpp"

namespace Geez {

    struct ConfigLoader {
    private:
        mINI::INIStructure content{};
        std::filesystem::path file;
        mINI::INIFile* cfg = nullptr;

        template <typename T>
        T convert_from_str(const std::string& str) {
            static_assert(false, "unsupported string conversion type...");
        }

    public:
        struct item_t {
            std::string key;
            std::string value;
        };

        struct section_t {
            std::string name;
            std::vector<item_t> items;
        };

        ConfigLoader() = delete;
        ConfigLoader(std::filesystem::path _file) : file(_file) {
            GZ_LOG(GZ_DEBUG, "Loading config file: %s", file.filename().string().c_str());
            
            cfg = new mINI::INIFile(file);
            if (!std::filesystem::exists(file)) {
                // Generate a new config
                GZ_LOG_T(GZ_OK, 1, "No geez.ini found, generating a default geez.ini");        
                if (!cfg->generate(content, true))
                    GZ_LOG_T(GZ_FAIL, 1, "Failed to generate geez.ini");
            }
            cfg->read(content);
        } 

        ~ConfigLoader() {
            GZ_LOG(GZ_DEBUG, "ConfigLoader quit");
            delete cfg;
        }

        void set_defaults(std::vector<section_t> config) {
            // load as Defaults
            GZ_LOG(GZ_DEBUG, "Setting config defaults...");
            for (section_t s : config) {
            
                content[s.name];
                for (item_t i : s.items) {
            
                    // Only set when not present in current file
                    if (content[s.name].has(i.key))
                        continue;

                    content[s.name].set(i.key, i.value);
                    GZ_LOG_T(GZ_OK, 1, "Section[%s] Key[%s] set to default", s.name.c_str(), i.key.c_str());
                }
            }
            cfg->write(content, true);
        }

        template <typename T>
        bool read_value(
            const std::string& section, const std::string& key, T& out) {
            
            if (content.has(section)) {
                if (content[section].has(key)) {
                    out = convert_from_str<T>(content[section][key]);
                    return true;
                }
                #ifdef GZ_BUILD_DEBUG
                else GZ_LOG(GZ_FAIL, "config section [%s] has no key [%s]", section, key);
                #endif
            }
            #ifdef GZ_BUILD_DEBUG
            else GZ_LOG(GZ_FAIL, "config has no section [%s]", section);
            #endif

            return false;
        }
    };

    template <>
    inline std::string ConfigLoader::convert_from_str<std::string>(const std::string& str) {
        return str;
    }

    template <>
    inline F32 ConfigLoader::convert_from_str<F32>(const std::string& str) {
        return std::stof(str);
    }

    template <>
    inline F64 ConfigLoader::convert_from_str<F64>(const std::string& str) {
        return std::stod(str);
    }

    template <>
    inline I32 ConfigLoader::convert_from_str<I32>(const std::string& str) {
        return std::stoi(str);
    }

    template <>
    inline U32 ConfigLoader::convert_from_str<U32>(const std::string& str) {
        return std::stoul(str);
    }
}

#endif