#ifndef GZ_RESOURCE_HPP
#define GZ_RESOURCE_HPP

#include "util/common_types.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace Geez
{
    using ResourceID = std::string;

    struct ResourceContributer
    {
        std::filesystem::path path;
        std::filesystem::file_time_type last_modified;
    };

    // Virtual Object
    struct IResource
    {
    friend struct ResourceManager;
    
    protected:
        ResourceID m_resource_id; 
        std::vector<ResourceContributer> m_contributers;

    public:
        virtual ~IResource() = default;
        
        inline ResourceID id() const noexcept { return m_resource_id; }

        // A contributer to a single resource i.e,. a file
        inline void add_contributer(const ResourceContributer& contributer) {
            m_contributers.push_back(contributer);
        }
    };
}

#endif