#ifndef GAME_H
#define GAME_H

global_constant os_key_code PAUSE_KEY = KeyCode_Escape;

//~ General status
enum lock_status {
    LockStatus_None,
    LockStatus_Permanent,
    
    LockStatus_TOTAL
};

enum repair_status {
    RepairStatus_None,
    RepairStatus_Broken,
    RepairStatus_Fixed,
    
    RepairStatus_TOTAL
};

enum quest_status {
    QuestStatus_None,
    QuestStatus_Active,
    QuestStatus_Completed,
};

//~ @quest_carillon_pages_h
struct carillon_pages_state {
    quest_status QuestStatus;
    b8 OrganWasRepaired;
    b8 BenchGhostIsFollowing;
};

//~ Entities
struct entity_ghost {
    asset_id Item;
    asset_id CurrentRoom;
};

//~ Game specific stuff
enum pray_status {
    PrayStatus_None,
    
    PrayStatus_First,
    PrayStatus_Other,
    
    PrayStatus_TOTAL,
};

enum murkwell_event_type {
    MurkwellEvent_None,
    MurkwellEvent_CarillonPages,
};

struct murkwell_event {
    murkwell_event_type Type;
};

//~ Declarations
internal b8 MurkwellStartCarillonPages(ta_system *TA, asset_system *Assets);

#endif //GAME_H