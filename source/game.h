#ifndef GAME_H
#define GAME_H

struct game_settings {
    os_key_code Jump        = KeyCode_Space;
    os_key_code Select      = (os_key_code)'X';
    os_key_code PlayerLeft  = KeyCode_Left;
    os_key_code PlayerRight = KeyCode_Right;
    
    os_key_code BoardUp    = KeyCode_Up;
    os_key_code BoardDown  = KeyCode_Down;
    os_key_code BoardLeft  = KeyCode_Left;
    os_key_code BoardRight = KeyCode_Right;
    os_key_code BoardPlace = KeyCode_Space;
};

global_constant os_key_code PAUSE_KEY = KeyCode_Escape;

//~ Text adventure stuff
global_constant u32 MAX_COMMAND_TOKENS = 64;
global_constant u32 TA_ROOM_DEFAULT_ITEM_COUNT = 8;
global_constant u32 INVENTORY_ITEM_COUNT = 10;

struct ta_system;
typedef void command_func(ta_system *TA, char **Words, u32 WordCount);

struct ta_string {
    asset_tag Tag;
    const char Data[];
};

struct ta_item {
    b8 Dirty;
    asset_tag Tag;
    u32 Cost;
    array<const char *> Aliases;
    array<const char *> Adjectives;
    array<ta_string *> Descriptions;
};

struct ta_room {
    b8 Dirty;
    const char *Name;
    asset_tag Tag;
    array<ta_string *> Descriptions;
    array<string> Items;
    string    Adjacents[Direction_TOTAL];
    asset_tag AdjacentTags[Direction_TOTAL];
};

struct ta_system {
    hash_table<const char *, command_func *> CommandTable;
    hash_table<const char *, direction> DirectionTable;
    
    hash_table<string, ta_room> RoomTable;
    hash_table<string, ta_item> ItemTable;
    ta_room *CurrentRoom;
    
    command_func *Callback;
    string_builder ResponseBuilder;
    
    array<string> Inventory;
    
    void Initialize(memory_arena *Arena);
    inline b8 AddItem(string Item);
    inline void ClearResponse();
    inline void Respond(const char *Format, ...);
    
    //~ Game specific data
    asset_tag_id OrganState;
    u32 Money;
};

#endif //GAME_H
