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
global u32 MAX_COMMAND_TOKENS = 64;

typedef void command_func(char **Words, u32 WordCount);

struct ta_room {
    const char *Name;
    const char *Description;
    string Adjacents[Direction_TOTAL];
    array<string> Items;
};

struct ta_system {
    hash_table<const char *, command_func *> CommandTable;
    hash_table<const char *, direction> DirectionTable;
    hash_table<string, ta_room>   RoomTable;
    ta_room *CurrentRoom;
    char ResponseBuffer[DEFAULT_BUFFER_SIZE];
    
    array<string> Inventory;
    
    void Initialize(memory_arena *Arena);
};

#endif //GAME_H
