#pragma once

#include "util/common_types.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

namespace Geez 
{
    struct ITagClient {
        U32  tag_id         = 0;
        bool tag_isModified = 0; // a callback is expected and will reset to false after action
    };

    // TagA, TagB, from (0=none 1=A 2=B 3=Both)
    using TagCallback = std::function<void(void*, void*, U8)>;

    struct TagResolver {

        // A connection is where any of the participating members action
        // will trigger a callback
        struct TagConnection {
            U32 a;
            U32 b;
            TagCallback cb;
        };

    private:
        std::unordered_map<U32, ITagClient const*> tag_map;
        std::vector<TagConnection> connections;
        
    public:

        TagResolver();
        ~TagResolver();

        inline void addTagClient(ITagClient const *client) {
            tag_map[client->tag_id] = client;
        }

        inline void addTagConnection(U32 id_a, U32 id_b, TagCallback callback) {
            connections.push_back({id_a, id_b, callback});
        }

        // TODO: Resolve_all_tag is called per post update
        
        inline void resolve_tag(U32 tag_id);
        inline void resolve_all_tag();
    };
}