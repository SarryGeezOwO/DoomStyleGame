#include "tag.hpp"

namespace Geez 
{
    TagResolver::TagResolver()
    {

    }

    TagResolver::~TagResolver()
    {

    }

    inline void TagResolver::resolve_tag(U32 tag_id)
    {
        // Resolves all tags that has connection to this specific tag_id
    }

    inline void TagResolver::resolve_all_tag()
    {
        for (TagConnection& conn : connections) {
            ITagClient const *a = tag_map[conn.a];
            ITagClient const *b = tag_map[conn.b];
            // if A and B triggers a callback on the same frame
            // one callback only gets triggered in response
            
            U8 from = a->tag_isModified + (b->tag_isModified * 2);
            
            if (a->tag_isModified) {
                conn.cb(reinterpret_cast<void*>(a), b, from);
            }
            else {
                if (b->tag_isModified)
                    conn.cb(a, b, from);
            }
        }   
    }
}