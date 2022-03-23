#ifndef GAME_H
#define GAME_H

global_constant os_key_code PAUSE_KEY = KeyCode_Escape;

//~ Theme
global_constant color BASE_BACKGROUND_COLOR = MakeColor(0x0a0d4aff);
global_constant color BASIC_COLOR      = MakeColor(0xf2f2f2ff);
global_constant color ROOM_TITLE_COLOR = MakeColor(0xff6969ff);
global_constant color ROOM_COLOR       = MakeColor(0xffe369ff);
global_constant color DIRECTION_COLOR  = MakeColor(0x84d197ff);
global_constant color ITEM_COLOR       = MakeColor(0x24e3e3ff);

global_constant color RESPONSE_COLOR = MakeColor(0x9063ffff);
global_constant color EMPHASIS_COLOR = MakeColor(0xe06ecdff);

global_constant fancy_font_format BasicFancy     = MakeFancyFormat(BASIC_COLOR, 0.0, 0.0, 0.0);
global_constant fancy_font_format RoomTitleFancy = MakeFancyFormat(ROOM_TITLE_COLOR, 1.0,  4.0, 2.0);
global_constant fancy_font_format ItemFancy      = MakeFancyFormat(ITEM_COLOR, 0.0,  0.0, 0.0);
global_constant fancy_font_format RoomFancy      = MakeFancyFormat(ROOM_COLOR, 1.0,  3.0, .125);
global_constant fancy_font_format DirectionFancy = MakeFancyFormat(DIRECTION_COLOR, 0.0,  0.0, 0.0);
global_constant fancy_font_format DescriptionFancies[] = {BasicFancy, DirectionFancy, RoomFancy, ItemFancy};



global_constant fancy_font_format ResponseFancy = MakeFancyFormat(RESPONSE_COLOR, 0.0,  0.0, 0.0);
global_constant fancy_font_format EmphasisFancy = MakeFancyFormat(EMPHASIS_COLOR, 1.0, 5.0, 3.0);


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
