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

typedef void command_func(char **Words, u32 WordCount);

enum ta_tag_id {
    TATag_None = 0,
    TATag_Play,
    TATag_Examine,
    TATag_Eat,
    TATag_Organ,
    TATag_Broken,
    TATag_Repaired,
    
    TATag_Count
};

struct ta_string {
    string Tag;
    const char Data[];
};

struct ta_item {
    string Tag;
    array<const char *> Aliases;
    array<ta_string *> Descriptions;
};

struct ta_room {
    const char *Name;
    string Tag;
    array<ta_string *> Descriptions;
    string Adjacents[Direction_TOTAL];
    array<string> Items;
};

struct ta_system {
    hash_table<const char *, command_func *> CommandTable;
    hash_table<const char *, direction> DirectionTable;
    
    hash_table<string, ta_room> RoomTable;
    hash_table<string, ta_item> ItemTable;
    ta_room *CurrentRoom;
    char ResponseBuffer[DEFAULT_BUFFER_SIZE];
    
    array<string> Inventory;
    
    void Initialize(memory_arena *Arena);
    inline b8 AddItem(string Item);
    inline void Respond(const char *Response);
    
    //~ Game specific data
    string OrganState;
};

#endif //GAME_H
