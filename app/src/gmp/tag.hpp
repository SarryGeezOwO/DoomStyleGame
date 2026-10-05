#ifndef GZ_TAG_HPP
#define GZ_TAG_HPP

#include "util/common_types.hpp"
#include "gmp_types.hpp"

#include "core/input.hpp"

#include <unordered_map>
#include <vector>
#include <queue>

namespace Geez {
    /*
        Overview of the system:
    */

    // Tag Operation Types
    // TAG_EVENT_*  can act as both Input and Output for a TagConnection

    #define TRIGGER_LIST \
        X(NO_TRIGGER) \
        X(SECTOR_CLOSED)    /*After Sector finished closing*/ \
        X(SECTOR_LIFTED)    /*After Sector finished lifting*/ \
        X(SECTOR_STAND)     /*Player present inside the sector*/ \
        X(WALL_ACTION)      /*Player Action button*/ \
        X(WALL_PASS)        /*Player walkthrough portal in any direction*/

    #define X(name) name,
    enum TagTrigger : U8 {
        TRIGGER_LIST
    };
    #undef X

    #define ACTION_LIST \
        X(NO_ACTION) \
        X(SECTOR_CLOSE)   /*CeilHeight -> FloorHeight*/ \
        X(SECTOR_LIFT)    /*FloorHeight -> CeilHeight*/ \
        X(SECTOR_OPEN)    /*Creates a gap between floor and ceil by drstp_open*/

    #define X(name) name,
    enum TagAction : U8 {
        ACTION_LIST
    };

    #undef X
    
    // These are special gates that bypasses input and output flags
    // like locking out the event until later on the story
    #define GZ_GMP_TAG_FLAG_NON_INTERACTABLE = 0x0001
    
    struct TagEvaluator {    
    private:
        static bool condition_for_wall(TagTrigger trigger, const wall_t *wall);
        static bool condition_for_sector(TagTrigger trigger, const sector_t* sector);

        static bool action_for_wall(TagAction action, wall_t *wall);
        static bool action_for_sector(TagAction action, sector_t *sector);
    
    public:
        struct RequiredSubSystems_t {
            Input *events;
        };

        static inline RequiredSubSystems_t required_subsystems{
            nullptr
        };
        
        static bool act(TagAction action, void *owner);
        static bool met(TagTrigger trigger, const void *owner);    
    };

    struct Tag {
        void *owner = nullptr; // Vague, yes but who do I care
        U16 id      = 0;
    };

    // This will serve as the router
    struct TagConnection {
        U16 from    = 0;
        U16 to      = 0;
        TagTrigger trigger = TagTrigger::NO_TRIGGER;
        TagAction  action  = TagAction::NO_ACTION;
    };

    // Handles dispatching, registration, and lookups.
    struct TagSystem {
    private:
        std::unordered_map<U16, Tag> tags;
        std::vector<TagConnection> conns;

        std::queue<TagConnection> pending;
        bool processing = false;

        Tag* get_tag(U16 tag_id);

    public:
        void register_tag(U16 tag_id, void *owner);
        void connect(U16 from, U16 to, TagTrigger trigger, TagAction action);

        // Process all tagConnection
        // by checking each Connections From inputFlags
        // and inserts that connection to the queue if inputs are satisfied
        // [Pre Update]
        void update();

        // [Post Update]
        void process_pending();
    };
}

#endif