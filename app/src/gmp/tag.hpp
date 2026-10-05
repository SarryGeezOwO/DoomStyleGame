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

    enum TagTrigger : U8 {
        NO_TRIGGER,               
        SECTOR_CLOSED,      // After Sector finished closing
        SECTOR_LIFTED,      // After Sector finished lifting
        SECTOR_STAND,       // Player present inside the sector
        WALL_ACTION,        // Player Action button
        WALL_PASS           // Player walkthrough portal in any direction
    };

    enum TagAction : U8 {
        NO_ACTION,
        SECTOR_CLOSE,       // CeilHeight -> FloorHeight
        SECTOR_LIFT         // FloorHeight -> CeilHeight
    };
    
    // These are special gates that bypasses input and output flags
    // like locking out the event until later on the story

    #define GZ_GMP_TAG_FLAG_NON_INTERACTABLE = 0x0001
    
    struct TagEvaluator {    
    private:
        static bool condition_for_wall(TagTrigger trigger, const wall_t *wall);
        static bool condition_for_sector(TagTrigger trigger, const sector_t* sector);

        static void action_for_wall(TagAction action, const wall_t *wall);
        static void action_for_sector(TagAction action, const sector_t *sector);
    
    public:
        struct RequiredSubSystems_t {
            Input *events;
        };

        static inline RequiredSubSystems_t required_subsystems{
            nullptr
        };
        
        static void act(TagAction action, const void *owner);
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