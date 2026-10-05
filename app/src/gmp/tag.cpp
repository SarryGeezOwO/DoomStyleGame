#include "tag.hpp"
#include "util/log.hpp"

namespace Geez {

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
            processing = false;
            return;
        }

        TagConnection con = pending.front();
        pending.pop();

        Tag *t = get_tag(con.to);
        if (!t) {
            processing = false; // Force next pending
            return;
        }
            
        TagEvaluator::act(con.action, t->owner);
    }




    void TagEvaluator::act(TagAction action, const void *owner) {
        switch (action)
        {
        case TagAction::SECTOR_CLOSE:
        case TagAction::SECTOR_LIFT:
            action_for_sector(action, reinterpret_cast<const sector_t*>(owner));
            break;
        
        default: break;
        }
    }

    bool TagEvaluator::met(TagTrigger trigger, const void *owner) {
        switch (trigger)
        {
        case TagTrigger::SECTOR_CLOSED:
        case TagTrigger::SECTOR_LIFTED:
        case TagTrigger::SECTOR_STAND:
            return condition_for_sector(trigger, reinterpret_cast<const sector_t*>(owner));

        case TagTrigger::WALL_ACTION:
        case TagTrigger::WALL_PASS:
            return condition_for_wall(trigger, reinterpret_cast<const wall_t*>(owner));

        default: break;
        }

        return false;
    }


    // ============= TRIGGERS ============= //


    #define EVAL_SYS TagEvaluator::required_subsystems

    bool wall_condition_action(const wall_t *wall) {
        return EVAL_SYS.events->check_key(SDLK_T, InputState::GZ_TAP);
    }

    bool wall_condition_pass(const wall_t *wall) {
        return false;
    }
    
    bool TagEvaluator::condition_for_wall(TagTrigger trigger, const wall_t *wall) {
        switch (trigger)
        {
        case TagTrigger::WALL_ACTION:   return wall_condition_action(wall);
        case TagTrigger::WALL_PASS:     return wall_condition_pass(wall);
        default: break;
        }
        return false;
    }





    bool TagEvaluator::condition_for_sector(TagTrigger trigger, const sector_t *sector) {
        return false;
    }

    

    // ============= ACTIONS ============= //
    
    
 
    
    void TagEvaluator::action_for_wall(TagAction action, const wall_t *wall) {
    }




    void sector_action_close(const sector_t *sector) {
        GZ_LOG(GZ_DEBUG, "Sector [%d] Closed or someshi", sector->id);
    }

    void sector_action_lift(const sector_t *sector) {
        GZ_LOG(GZ_DEBUG, "Sector [%d] lifted or someshi", sector->id);
    }

    void TagEvaluator::action_for_sector(TagAction action, const sector_t *sector) {
        switch (action)
        {
        case TagAction::SECTOR_CLOSE: sector_action_close(sector); break;
        case TagAction::SECTOR_LIFT:  sector_action_lift(sector); break;
        
        default:
            break;
        }
    }
    
}
