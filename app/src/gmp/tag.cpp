#include "tag.hpp"
#include "util/log.hpp"
#include "util/utility.hpp"

namespace Geez {

    #define X(name) #name,
    static const char* const trigger_names[] = { TRIGGER_LIST };
    static const char* const action_names[]  = { ACTION_LIST };
    #undef X

    namespace Internal {
        const char* to_string(TagTrigger t) {
            return (t < std::size(trigger_names)) ? trigger_names[t] : "INVALID_TRIGGER";
        }

        const char* to_string(TagAction a) {
            return (a < std::size(action_names)) ? action_names[a] : "INVALID_ACTION";
        }

        bool to_enum(const char* name, TagTrigger& trigger) { return lookup(name, trigger_names, trigger); }
        bool to_enum(const char* name, TagAction&  action)  { return lookup(name, action_names,  action);  }
    }

    Tag *TagSystem::get_tag(U16 tag_id) {
        return (tags.find(tag_id) != tags.end() ? &tags.at(tag_id) : nullptr);
    }

    void TagSystem::register_tag(U16 tag_id, void *owner) {
        if (tag_id == 0) {
            GZ_LOG(GZ_FAIL, "Tag Id should be non-zero");
            return;
        }
        #ifdef GZ_BUILD_DEBUG
            if (tags.find(tag_id) != tags.end())
                GZ_LOG(GZ_WARNING, "Tag silent overwrite [ID: %d]", tag_id);
        #endif
        tags[tag_id].id     = tag_id;
        tags[tag_id].owner  = owner;
        GZ_LOG(GZ_OK, "Tag registered [%d]", tag_id);
    }

    void TagSystem::connect(U16 from, U16 to, TagTrigger trigger, TagAction action) {
        conns.push_back({from, to, trigger, action});
        GZ_LOG(GZ_OK, "Tag Connection between [%d] -> [%d]", from, to);
    }

    void TagSystem::update() {
        for (TagConnection& con : conns) {
            Tag *t = get_tag(con.from);
            if (!t) continue;
            
            if (TagEvaluator::met(con.trigger, t->owner)) {
                pending.push(con);
            }
        }
    }

    void TagSystem::process_pending() {
        if (pending.empty()) {
            return;
        }

        TagConnection con = pending.front();
        Tag *t = get_tag(con.to);

        if (t) {
            // True means action is done
            if (TagEvaluator::act(con.action, t->owner)) {
                pending.pop();
                GZ_LOG(GZ_SUCCESS, "Action completed... doingNext action");
            }
        }
        else {
            pending.pop();
            GZ_LOG(GZ_FAIL, "NULL Tag, process_pending()...");
        }
    }


    #define EVAL_SYS TagEvaluator::required_subsystems
    // ============= ACTIONS ============= //

    bool action_NO_ACTION(void *owner) { return true; }

    bool action_SECTOR_CLOSE(void *owner) {
        sector_t *sector = reinterpret_cast<sector_t*>(owner);
        if (sector->ceil_height > sector->floor_height) 
            sector->ceil_height -= 0.01f;
        return (sector->ceil_height <= sector->floor_height);
    }

    bool action_SECTOR_LIFT(void *owner) {
        sector_t *sector = reinterpret_cast<sector_t*>(owner);
        if (sector->floor_height < sector->ceil_height) 
            sector->floor_height += 0.01f;
        return (sector->floor_height >= sector->ceil_height);
    }

    bool action_SECTOR_OPEN(void *owner) {
        sector_t *sector = reinterpret_cast<sector_t*>(owner);
        F32 gap = abs(sector->ceil_height - sector->floor_height);
        if (gap < sector->drstp_open) 
            sector->ceil_height += 0.01f;
        return (gap >= sector->drstp_open);
    }

    bool TagEvaluator::act(TagAction action, void *owner) {
        #define X(name) case TagAction::name: return action_##name(owner);
        switch (action)
        {
            ACTION_LIST
            default: return true;
        }
        #undef X
    }

    // ============= ---END--- ============= //

    // ============= TRIGGERS ============= //

    bool trigger_NO_TRIGGER(const void *owner) { return false; }

    bool trigger_SECTOR_CLOSED(const void *owner) { 
        return false;
    }

    bool trigger_SECTOR_LIFTED(const void *owner) { 
        return false; 
    }

    bool trigger_SECTOR_STAND(const void *owner) { 
        return false; 
    }

    bool trigger_WALL_ACTION(const void *owner) { 
        return EVAL_SYS.events->check_key(SDLK_T, InputState::GZ_TAP);
    }
    
    bool trigger_WALL_PASS(const void *owner) { 
        return EVAL_SYS.events->check_key(SDLK_Y, InputState::GZ_TAP);
    }

    bool TagEvaluator::met(TagTrigger trigger, const void *owner) {
        #define X(name) case TagTrigger::name: return trigger_##name(owner);
        switch (trigger)
        {
            TRIGGER_LIST
            default: return false;
        }
        #undef X
    }

    // ============= ---END--- ============= //
}
