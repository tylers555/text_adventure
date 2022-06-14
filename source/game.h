#ifndef GAME_H
#define GAME_H

global_constant os_key_code PAUSE_KEY = KeyCode_Escape;

//~ Entities
struct entity_ghost {
    ta_id Item;
    ta_id CurrentRoom;
};

//~ Game specific stuff
enum pray_state {
    PrayState_None,
    
    PrayState_First,
    PrayState_Other,
    
    PrayState_TOTAL,
};

enum murkwell_event_type {
    MurkwellEvent_None,
    MurkwellEvent_CarillonPages,
};

struct murkwell_event {
    murkwell_event_type Type;
    
};

#endif //GAME_H